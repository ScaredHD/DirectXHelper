#pragma once


#include "PCH.h"

namespace dxh
{


class DescriptorHeap
{
public:
  DescriptorHeap(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_DESC desc);

  size_t Count() const { return count; }

  CD3DX12_CPU_DESCRIPTOR_HANDLE CPUHandle(INT index);

  CD3DX12_GPU_DESCRIPTOR_HANDLE GPUHandle(INT index);

  ID3D12DescriptorHeap* Heap() const { return heap.Get(); }

private:
  D3D12_DESCRIPTOR_HEAP_TYPE type;
  size_t count = 0;
  UINT incrementSize = 0;
  Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap;
};

std::unique_ptr<DescriptorHeap> MakeRTVHeap(ID3D12Device* device, UINT count);

std::unique_ptr<DescriptorHeap> MakeDSVHeap(ID3D12Device* device, UINT count);

std::unique_ptr<DescriptorHeap> MakeCbvSrvUavHeap(ID3D12Device* device, UINT count);

std::unique_ptr<DescriptorHeap> MakeSamplerHeap(ID3D12Device* device, UINT count);

}  // namespace dxh