#include "DescriptorHeap.h"


namespace dxh
{


DescriptorHeap* DescriptorHeapPool::RequestHeap()
{
  if (currentHeap) {
    return currentHeap.get();
  }

  if (!retiredHeaps.empty()) {
    RetiredHeap& h = retiredHeaps.front();
    // TODO:
    uint64_t completedFence;
    ;
    if (completedFence >= h.fenceValue) {
      availableHeaps.push(std::move(h.heap));
      retiredHeaps.pop();
    }
  }

  if (!availableHeaps.empty()) {
    currentHeap = std::move(availableHeaps.front());
    availableHeaps.pop();
  } else {
    currentHeap = std::make_unique<DescriptorHeap>(
      device.Get(),
      D3D12_DESCRIPTOR_HEAP_DESC{type, descriptorsPerHeap, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, 0}
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


void DynamicDescriptorHeap::ParseRootSignature(const RootSignature& rootSignature)
{
  cache.rootIndexToTable.clear();

  for (int i = 0; i < rootSignature.ParameterCount(); ++i) {
    const CD3DX12_ROOT_PARAMETER1& param = rootSignature.Parameter(i);
    if (param.ParameterType != D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE) {
      continue;
    }
    auto rangeCount = param.DescriptorTable.NumDescriptorRanges;
    int descriptorCount = 0;
    for (int j = 0; j < rangeCount; ++j) {
      const D3D12_DESCRIPTOR_RANGE1& range = param.DescriptorTable.pDescriptorRanges[j];
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

void DynamicDescriptorHeap::BindModifiedDescriptors(ID3D12GraphicsCommandList* commandList)
{
  size_t descriptorsToAlloc = cache.StagedDescriptorCount();
  if (descriptorsToAlloc > heapPool.AvailableDescriptorCount()) {
    heapPool.RetireHeap();
    heapPool.RequestHeap();
  }


  for (const auto& [rootIndex, entry] : cache.rootIndexToTable) {
    if (!entry.dirty) {
      continue;
    }
  }
}

}  // namespace dxh