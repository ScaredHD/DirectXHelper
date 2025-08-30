#include "DescriptorHeap.h"


namespace dxh
{


DescriptorHeap* DescriptorHeapPool::RequestHeap()
{
  if (currentHeap) {
    return currentHeap.get();
  }

  if (!availableHeaps.empty()) {
    currentHeap = std::move(availableHeaps.front());
    availableHeaps.pop();
  } else {
    currentHeap = std::make_unique<DescriptorHeap>(
      device, D3D12_DESCRIPTOR_HEAP_DESC{type, descriptorsPerHeap, flag, 0}
    );
  }

  nextAvailableIndex = 0;
  return currentHeap.get();
}

DescriptorHeap* DescriptorHeapPool::CurrentHeap()
{
  if (!currentHeap) {
    return RequestHeap();
  }
  return currentHeap.get();
}

void DescriptorHeapPool::RetireHeap()
{
  if (!currentHeap) {
    return;
  }

  RetiredHeap retired;
  retired.heap = std::move(currentHeap);
  retired.fenced = false;
  retiredHeaps.push(std::move(retired));
  currentHeap.reset();
}

void DescriptorHeapPool::RenewHeaps(ID3D12Fence* fence)
{
  uint64_t completedFence = fence->GetCompletedValue();
  for (auto& r = retiredHeaps.front(); r.fenced && r.fenceValue <= completedFence;
       retiredHeaps.pop()) {
    availableHeaps.push(std::move(r.heap));
  }
}

void DescriptorHeapPool::MarkRetiredHeaps(uint64_t fenceValue)
{
  std::queue<RetiredHeap> updated;
  while (!retiredHeaps.empty()) {
    auto& r = retiredHeaps.front();
    if (!r.fenced) {
      r.fenceValue = fenceValue;
      r.fenced = true;
    }
    updated.push(std::move(r));
    retiredHeaps.pop();
  }
  retiredHeaps = std::move(updated);
}

void DynamicDescriptorHeap::ParseRootSignature(const RootSignature& rootSignature)
{
  cache.rootIndexToTable.clear();

  for (size_t i = 0; i < rootSignature.ParameterCount(); ++i) {
    const D3D12_ROOT_PARAMETER& param = rootSignature.Parameter(i);
    if (param.ParameterType != D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE) {
      continue;
    }
    auto rangeCount = param.DescriptorTable.NumDescriptorRanges;
    size_t descriptorCount = 0;
    for (size_t j = 0; j < rangeCount; ++j) {
      const D3D12_DESCRIPTOR_RANGE& range = param.DescriptorTable.pDescriptorRanges[j];
      descriptorCount += range.NumDescriptors;
    }
    DescriptorTableCache::TableEntry entry{};
    entry.dirty = false;
    entry.handles.resize(descriptorCount);
    cache.rootIndexToTable[i] = std::move(entry);
  }
}

void DynamicDescriptorHeap::SetDescriptors(
  UINT rootIndex,
  UINT offset,
  UINT count,
  const D3D12_CPU_DESCRIPTOR_HANDLE handles[]
)
{
  assert(cache.rootIndexToTable.count(rootIndex));

  DescriptorTableCache::TableEntry& entry = cache.rootIndexToTable[rootIndex];
  assert(offset < entry.DescriptorCount());
  assert(offset + count <= entry.DescriptorCount());
  entry.dirty = true;
  std::copy(handles, handles + count, std::begin(entry.handles) + offset);
}

void DynamicDescriptorHeap::BindModifiedDescriptors(
  ID3D12Device* device,
  ID3D12GraphicsCommandList* commandList
)
{
  if (!cache.IsDirty()) {
    return;
  }

  size_t descriptorsToAlloc = cache.StagedDescriptorCount();
  assert(
    descriptorsToAlloc <= heapPool.MaxDescriptorsPerHeap() &&
    "Descriptors to allocate are more than available even for newly requested heap"
  );

  if (!heapPool.CanAllocate(descriptorsToAlloc)) {
    cache.MarkAllDirty();
    heapPool.RetireHeap();
    heapPool.RequestHeap();
    BindModifiedDescriptors(device, commandList);
    return;
  }

  commandList->SetDescriptorHeaps(1, heapPool.CurrentHeap()->HeapAddress());

  for (auto& [rootIndex, entry] : cache.rootIndexToTable) {
    if (!entry.dirty) {
      continue;
    }

    // Copy descriptors in dirty entry to newly allocated space in heap
    auto extraDescriptorCount = entry.DescriptorCount();
    UINT baseIndex = heapPool.NextAvailableIndex();
    DescriptorHeap* currentHeap = heapPool.CurrentHeap();
    const auto& srcDescriptors = entry.handles;

    for (size_t i = 0; i < extraDescriptorCount; ++i) {
      device->CopyDescriptorsSimple(
        1, currentHeap->CPUHandle(baseIndex + i), srcDescriptors[i], currentHeap->Type()
      );
    }

    commandList->SetGraphicsRootDescriptorTable(rootIndex, currentHeap->GPUHandle(baseIndex));
  }
}


}  // namespace dxh