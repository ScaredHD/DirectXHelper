#include "Views.h"

namespace dxh
{


std::unique_ptr<RenderTargetView>
CreateDefaultRTVForTexture2D(ID3D12Device* device, ID3D12Resource* resource, DXGI_FORMAT format, D3D12_CPU_DESCRIPTOR_HANDLE outCPUHandle)
{
  D3D12_RENDER_TARGET_VIEW_DESC desc{};
  desc.Format = format;
  desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
  desc.Texture2D.PlaneSlice = 0;
  desc.Texture2D.MipSlice = 0;
  return std::make_unique<RenderTargetView>(device, resource, desc, outCPUHandle);
}

std::unique_ptr<DepthStencilView> CreateDefaultDSVForTexture2D(
  ID3D12Device* device,
  ID3D12Resource* resource,
  DXGI_FORMAT format,
  D3D12_CPU_DESCRIPTOR_HANDLE outCPUHandle
)
{
    D3D12_DEPTH_STENCIL_VIEW_DESC desc{};
    desc.Format = format;
    desc.Flags = D3D12_DSV_FLAG_NONE;
    desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    return std::make_unique<DepthStencilView>(device, resource, desc, outCPUHandle);
}

std::unique_ptr<ShaderResourceView>
CreateDefaultSRVForTexture2D(ID3D12Device* device, ID3D12Resource* resource, DXGI_FORMAT format, D3D12_CPU_DESCRIPTOR_HANDLE outCPUHandle)
{
  D3D12_SHADER_RESOURCE_VIEW_DESC desc{};
  desc.Format = format;
  desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
  desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
  desc.Texture2D.MostDetailedMip = 0;
  desc.Texture2D.MipLevels = 1;
  desc.Texture2D.PlaneSlice = 0;
  desc.Texture2D.ResourceMinLODClamp = 0.f;
  return std::make_unique<ShaderResourceView>(device, resource, desc, outCPUHandle);
}

std::unique_ptr<UnorderedAccessView>
CreateDefaultUAVForTexture2D(ID3D12Device* device, ID3D12Resource* resource, DXGI_FORMAT format, D3D12_CPU_DESCRIPTOR_HANDLE outCPUHandle)
{
  D3D12_UNORDERED_ACCESS_VIEW_DESC desc{};
  desc.Format = format;
  desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
  desc.Texture2D.MipSlice = 0;
  desc.Texture2D.PlaneSlice = 0;
  return std::make_unique<UnorderedAccessView>(device, resource, desc, outCPUHandle);
}


}  // namespace dxh