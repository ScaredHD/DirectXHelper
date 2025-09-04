
#pragma once

#include "PCH.h"


namespace dxh
{

class Buffer;
class Texture;

template<typename ElemType, size_t alignment>
class UploadHeapArray;

class Device
{
public:
  explicit Device(IDXGIFactory4* factory);

  ID3D12Device* Get() const { return device.Get(); }
  Microsoft::WRL::ComPtr<ID3D12Device> GetComPtr() const { return device; }

  void CreateCBV(const dxh::Buffer& buffer, D3D12_CPU_DESCRIPTOR_HANDLE handle);
  void CreateSRV(const dxh::Texture& texture, D3D12_CPU_DESCRIPTOR_HANDLE handle);

  template<typename ElemType, size_t alignment>
  void CreateSRV(
    const dxh::UploadHeapArray<ElemType, alignment>& buffer,
    D3D12_CPU_DESCRIPTOR_HANDLE handle
  )
  {
    D3D12_SHADER_RESOURCE_VIEW_DESC desc{};
    desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    desc.Format = DXGI_FORMAT_UNKNOWN;
    desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    desc.Buffer.FirstElement = 0;
    desc.Buffer.NumElements = static_cast<UINT>(buffer.ElementCount());
    desc.Buffer.StructureByteStride = static_cast<UINT>(buffer.ElementPaddedSize());
    desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
    device->CreateShaderResourceView(buffer.Resource(), &desc, handle);
  }

  void CreateUAV(const dxh::Texture& texture, D3D12_CPU_DESCRIPTOR_HANDLE handle);
  void CreateDSV(const dxh::Texture& texture, D3D12_CPU_DESCRIPTOR_HANDLE handle);


private:
  Microsoft::WRL::ComPtr<ID3D12Device> device;
};

}  // namespace dxh
