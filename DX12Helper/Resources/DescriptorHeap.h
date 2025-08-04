#pragma once


#include "PCH.h"

namespace dxh
{


template<size_t count>
class DescriptorHeap
{
public:
  DescriptorHeap(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_DESC desc) : type{desc.Type}
  {
    incrementSize = device->GetDescriptorHandleIncrementSize(type);
    DX::ThrowIfFailed(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(heap.GetAddressOf())));
  }

  size_t Count() const { return count; }

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
  D3D12_DESCRIPTOR_HEAP_TYPE type;
  UINT incrementSize = 0;
  Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap;
};




}  // namespace dxh