#pragma once

#include "PCH.h"

namespace dxh
{

class ResourceView
{
public:
  explicit ResourceView(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle) : cpuHandle_{cpuHandle} {}

  D3D12_CPU_DESCRIPTOR_HANDLE CPUHandle() const { return cpuHandle_; }

private:
  D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle_;
};

class RenderTargetView : public ResourceView
{
public:
  explicit RenderTargetView(
    ID3D12Device* device,
    ID3D12Resource* resource,
    D3D12_RENDER_TARGET_VIEW_DESC desc,
    D3D12_CPU_DESCRIPTOR_HANDLE outCPUHandle
  )
      : ResourceView{outCPUHandle}
  {
    device->CreateRenderTargetView(resource, &desc, CPUHandle());
  }
};

class DepthStencilView : public ResourceView
{
public:
  explicit DepthStencilView(
    ID3D12Device* device,
    ID3D12Resource* resource,
    D3D12_DEPTH_STENCIL_VIEW_DESC desc,
    D3D12_CPU_DESCRIPTOR_HANDLE outCPUHandle
  )
      : ResourceView{outCPUHandle}
  {
    device->CreateDepthStencilView(resource, &desc, CPUHandle());
  }
};

class UnorderedAccessView : public ResourceView
{
public:
  explicit UnorderedAccessView(
    ID3D12Device* device,
    ID3D12Resource* resource,
    D3D12_UNORDERED_ACCESS_VIEW_DESC desc,
    D3D12_CPU_DESCRIPTOR_HANDLE outCPUHandle
  )
      : ResourceView{outCPUHandle}
  {
    device->CreateUnorderedAccessView(resource, nullptr, &desc, CPUHandle());
  }
};

class ShaderResourceView : public ResourceView
{
public:
  explicit ShaderResourceView(
    ID3D12Device* device,
    ID3D12Resource* resource,
    D3D12_SHADER_RESOURCE_VIEW_DESC desc,
    D3D12_CPU_DESCRIPTOR_HANDLE outCPUHandle
  )
      : ResourceView{outCPUHandle}
  {
    device->CreateShaderResourceView(resource, &desc, CPUHandle());
  }
};

class ConstantBufferView : public ResourceView
{
public:
  explicit ConstantBufferView(
    ID3D12Device* device,
    D3D12_CONSTANT_BUFFER_VIEW_DESC desc,
    D3D12_CPU_DESCRIPTOR_HANDLE outCPUHandle
  )
      : ResourceView{outCPUHandle}
  {
    device->CreateConstantBufferView(&desc, CPUHandle());
  }

  explicit ConstantBufferView(
    ID3D12Device* device,
    D3D12_GPU_VIRTUAL_ADDRESS gpuVirtualAddress,
    UINT byteSize,
    D3D12_CPU_DESCRIPTOR_HANDLE outCPUHandle
  )
      : ResourceView{outCPUHandle}
  {
    D3D12_CONSTANT_BUFFER_VIEW_DESC desc{};
    desc.BufferLocation = gpuVirtualAddress;
    desc.SizeInBytes = byteSize;
    device->CreateConstantBufferView(&desc, CPUHandle());
  }
};

std::unique_ptr<RenderTargetView> CreateDefaultRTVForTexture2D(
  ID3D12Device* device,
  ID3D12Resource* resource,
  DXGI_FORMAT format,
  D3D12_CPU_DESCRIPTOR_HANDLE outCPUHandle
);

std::unique_ptr<DepthStencilView> CreateDefaultDSVForTexture2D(
  ID3D12Device* device,
  ID3D12Resource* resource,
  DXGI_FORMAT format,
  D3D12_CPU_DESCRIPTOR_HANDLE outCPUHandle
);

std::unique_ptr<ShaderResourceView> CreateDefaultSRVForTexture2D(
  ID3D12Device* device,
  ID3D12Resource* resource,
  DXGI_FORMAT format,
  D3D12_CPU_DESCRIPTOR_HANDLE outCPUHandle
);

std::unique_ptr<UnorderedAccessView> CreateDefaultUAVForTexture2D(
  ID3D12Device* device,
  ID3D12Resource* resource,
  DXGI_FORMAT format,
  D3D12_CPU_DESCRIPTOR_HANDLE outCPUHandle
);


}  // namespace dxh
