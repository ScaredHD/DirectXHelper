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
    D3D12_DESCRIPTOR_HEAP_FLAGS flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE
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

  CD3DX12_CPU_DESCRIPTOR_HANDLE CPUHandle(INT index)
  {
    return {heap->GetCPUDescriptorHandleForHeapStart(), index, incrementSize};
  }


  CD3DX12_GPU_DESCRIPTOR_HANDLE GPUHandle(INT index)
  {
    return {heap->GetGPUDescriptorHandleForHeapStart(), index, incrementSize};
  }


  ID3D12DescriptorHeap* Heap() const { return heap.Get(); }

private:
  UINT incrementSize = 0;
  D3D12_DESCRIPTOR_HEAP_DESC desc = {};
  Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap;
};

class DescriptorHeapPool
{
public:
  DescriptorHeapPool(
    const Microsoft::WRL::ComPtr<ID3D12Device>& device,
    D3D12_DESCRIPTOR_HEAP_TYPE type,
    UINT descriptorsPerHeap = 1024
  )
      : device{device},
        type{type},
        descriptorsPerHeap{descriptorsPerHeap}
  {
  }

  DescriptorHeap* RequestHeap();

  DescriptorHeap* CurrentHeap();

  void RetireHeap();

  void UpdateRetiredHeaps();

  void MarkRetiredHeaps(uint64_t fenceValue);

  UINT NextAvailableIndex() const { return nextAvailableIndex; }

  UINT AvailableDescriptorCount() const
  {
    return currentHeap ? currentHeap->Count() - nextAvailableIndex : 0;
  }

private:
  Microsoft::WRL::ComPtr<ID3D12Device> device;
  D3D12_DESCRIPTOR_HEAP_TYPE type;
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

  size_t currentCapacity = 0;
};


struct DescriptorTableCache {

  struct TableEntry {
    bool dirty;
    std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> handles;

    size_t DescriptorCount() const { return handles.size(); }
  };

  std::unordered_map<int, TableEntry> rootIndexToTable;

  size_t StagedDescriptorCount() const
  {
    return std::accumulate(
      std::begin(rootIndexToTable), std::end(rootIndexToTable), 0,
      [](const auto& p0, const auto& p1) {
        return (p0.second.dirty ? p0.second.DescriptorCount() : 0) +
               (p1.second.dirty ? p1.second.DescriptorCount() : 0);
      }
    );
  }

  void Clear() { rootIndexToTable.clear(); }

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
  void ParseRootSignature(const RootSignature& rootSignature);

  void SetDescriptors(
    UINT rootIndex,
    UINT offset,
    UINT count,
    const D3D12_CPU_DESCRIPTOR_HANDLE handles[]
  );

  void BindModifiedDescriptors(ID3D12GraphicsCommandList* commandList);

private:
  DescriptorHeapPool heapPool;
  DescriptorTableCache cache;
};


}  // namespace dxh