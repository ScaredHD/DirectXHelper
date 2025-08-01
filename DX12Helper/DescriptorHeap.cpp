#include "DescriptorHeap.h"


namespace dxh
{


DescriptorHeap::DescriptorHeap(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_DESC desc)
    : type{desc.Type},
      count{desc.NumDescriptors}
{
  incrementSize = device->GetDescriptorHandleIncrementSize(type);
  DX::ThrowIfFailed(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(heap.GetAddressOf())));
}

CD3DX12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::CPUHandle(INT index)
{
  return {heap->GetCPUDescriptorHandleForHeapStart(), index, incrementSize};
}

CD3DX12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::GPUHandle(INT index)
{
  return {heap->GetGPUDescriptorHandleForHeapStart(), index, incrementSize};
}

std::unique_ptr<DescriptorHeap> MakeRTVHeap(ID3D12Device* device, UINT count)
{
  D3D12_DESCRIPTOR_HEAP_DESC desc{};
  desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
  desc.NumDescriptors = count;
  desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

  return std::make_unique<DescriptorHeap>(device, desc);
}

std::unique_ptr<DescriptorHeap> MakeDSVHeap(ID3D12Device* device, UINT count)
{
  D3D12_DESCRIPTOR_HEAP_DESC desc{};
  desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
  desc.NumDescriptors = count;
  desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
  desc.NodeMask = 0;

  return std::make_unique<DescriptorHeap>(device, desc);
}

std::unique_ptr<DescriptorHeap> MakeCbvSrvUavHeap(ID3D12Device* device, UINT count)
{
  D3D12_DESCRIPTOR_HEAP_DESC desc{};
  desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
  desc.NumDescriptors = count;
  desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
  // desc.NodeMask = 0;

  return std::make_unique<DescriptorHeap>(device, desc);
}

std::unique_ptr<DescriptorHeap> MakeSamplerHeap(ID3D12Device* device, UINT count)
{
  D3D12_DESCRIPTOR_HEAP_DESC desc{};
  desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
  desc.NumDescriptors = count;
  desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
  // desc.NodeMask = 0;

  return std::make_unique<DescriptorHeap>(device, desc);
}

}  // namespace dxh