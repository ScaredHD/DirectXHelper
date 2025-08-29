#pragma once


#include <queue>
#include <unordered_map>
#include <utility>

#include "PCH.h"
#include "RootSignature.h"


namespace dxh
{

class DescriptorHeap
{
public:
  explicit DescriptorHeap(
    ID3D12Device* device,
    D3D12_DESCRIPTOR_HEAP_TYPE type,
    UINT count,
    D3D12_DESCRIPTOR_HEAP_FLAGS flags
  )
      : DescriptorHeap{device, D3D12_DESCRIPTOR_HEAP_DESC{type, count, flags, 0}}
  {
  }

  explicit DescriptorHeap(ID3D12Device* device, const D3D12_DESCRIPTOR_HEAP_DESC& desc)
      : incrementSize{device->GetDescriptorHandleIncrementSize(desc.Type)},
        desc{desc}
  {
    DX::ThrowIfFailed(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(heap.GetAddressOf())));
  }

  D3D12_DESCRIPTOR_HEAP_TYPE Type() const { return desc.Type; }
  UINT Count() const { return desc.NumDescriptors; }

  CD3DX12_CPU_DESCRIPTOR_HANDLE CPUHandle(UINT index)
  {
    return {heap->GetCPUDescriptorHandleForHeapStart(), static_cast<INT>(index), incrementSize};
  }


  CD3DX12_GPU_DESCRIPTOR_HANDLE GPUHandle(UINT index)
  {
    return {heap->GetGPUDescriptorHandleForHeapStart(), static_cast<INT>(index), incrementSize};
  }


  ID3D12DescriptorHeap* Heap() const { return heap.Get(); }

  ID3D12DescriptorHeap** HeapAddress() { return heap.GetAddressOf(); }

private:
  UINT incrementSize = 0;
  D3D12_DESCRIPTOR_HEAP_DESC desc = {};
  Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap;
};

template<
  D3D12_DESCRIPTOR_HEAP_TYPE heapType,
  D3D12_DESCRIPTOR_HEAP_FLAGS flag,
  UINT descriptorsPerHeap = 1024>
class DescriptorPool
{
public:
  explicit DescriptorPool(ID3D12Device* device)
      : device{device},
        incrementSize{device->GetDescriptorHandleIncrementSize(heapType)}
  {
  }

  CD3DX12_CPU_DESCRIPTOR_HANDLE Allocate(UINT count = 1)
  {
    if (heaps.empty() || AvailableDescriptorCount() < count) {
      RequestHeap();
    }

    CD3DX12_CPU_DESCRIPTOR_HANDLE handle = heaps.back().CPUHandle(nextAvailableIndex);
    nextAvailableIndex += count;

    return handle;
  }

  void FreeAll()
  {
    heaps.clear();
    nextAvailableIndex = 0;
  }

private:
  void RequestHeap()
  {
    heaps.emplace_back(device, heapType, descriptorsPerHeap, flag);
    nextAvailableIndex = 0;
  }

  UINT AvailableDescriptorCount() const
  {
    return heaps.empty() ? 0 : heaps.back().Count() - nextAvailableIndex;
  }


  ID3D12Device* device = nullptr;
  UINT incrementSize = 0;
  std::vector<DescriptorHeap> heaps;
  UINT nextAvailableIndex = 0;
};

using CbvSrvUavPool =
  DescriptorPool<D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE>;
using RTVPool = DescriptorPool<D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE>;
using DSVPool = DescriptorPool<D3D12_DESCRIPTOR_HEAP_TYPE_DSV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE>;

class DescriptorHeapPool
{
public:
  DescriptorHeapPool(
    ID3D12Device* device,
    D3D12_DESCRIPTOR_HEAP_TYPE type,
    D3D12_DESCRIPTOR_HEAP_FLAGS flag,
    UINT descriptorsPerHeap = 1024
  )
      : device{device},
        type{type},
        flag{flag},
        descriptorsPerHeap{descriptorsPerHeap}
  {
  }

  DescriptorHeap* RequestHeap();

  DescriptorHeap* CurrentHeap();

  void RetireHeap();

  void RenewHeaps(ID3D12Fence* fence);

  void MarkRetiredHeaps(uint64_t fenceValue);

  UINT NextAvailableIndex() const { return nextAvailableIndex; }

  UINT AvailableDescriptorCount() const
  {
    return currentHeap ? currentHeap->Count() - nextAvailableIndex : 0;
  }

  bool CanAllocate(UINT count) const { return count <= AvailableDescriptorCount(); }

  D3D12_CPU_DESCRIPTOR_HANDLE Allocate(UINT count)
  {
    assert(CanAllocate(count));

    if (!currentHeap) {
      RequestHeap();
    }

    D3D12_CPU_DESCRIPTOR_HANDLE handle = currentHeap->CPUHandle(nextAvailableIndex);
    nextAvailableIndex += count;

    return handle;
  }

  UINT MaxDescriptorsPerHeap() const { return descriptorsPerHeap; }

private:
  ID3D12Device* device;
  D3D12_DESCRIPTOR_HEAP_TYPE type;
  D3D12_DESCRIPTOR_HEAP_FLAGS flag;
  UINT descriptorsPerHeap = 1024;

  std::unique_ptr<DescriptorHeap> currentHeap;
  UINT nextAvailableIndex = 0;

  struct RetiredHeap {
    std::unique_ptr<DescriptorHeap> heap;
    uint64_t fenceValue;
    bool fenced = false;
  };

  std::queue<RetiredHeap> retiredHeaps;
  std::queue<std::unique_ptr<DescriptorHeap>> availableHeaps;
};

struct DescriptorTableCache {

  struct TableEntry {
    bool dirty;
    std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> handles;
    size_t DescriptorCount() const { return handles.size(); }
  };

  std::unordered_map<size_t, TableEntry> rootIndexToTable;

  size_t StagedDescriptorCount() const
  {
    return std::accumulate(
      std::begin(rootIndexToTable), std::end(rootIndexToTable), 0, [](size_t sum, const auto& p) {
        return sum + (p.second.dirty ? p.second.DescriptorCount() : 0);
      }
    );
  }

  void Clear() { rootIndexToTable.clear(); }

  bool IsDirty() const
  {
    return std::any_of(std::begin(rootIndexToTable), std::end(rootIndexToTable), [](const auto& p) {
      return p.second.dirty;
    });
  }

  void MarkAllDirty()
  {
    for (auto& [rootIndex, entry] : rootIndexToTable) {
      entry.dirty = true;
    }
  }
};

class DynamicDescriptorHeap
{
public:
  DynamicDescriptorHeap(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type)
      : heapPool{device, type, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE}
  {
  }

  void ParseRootSignature(const RootSignature& rootSignature);

  void SetDescriptors(
    UINT rootIndex,
    UINT offset,
    UINT count,
    const D3D12_CPU_DESCRIPTOR_HANDLE handles[]
  );


  void BindModifiedDescriptors(ID3D12Device* device, ID3D12GraphicsCommandList* commandList);

private:
  DescriptorHeapPool heapPool;
  DescriptorTableCache cache;
};


}  // namespace dxh