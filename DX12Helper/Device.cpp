#include "Device.h"

#include "Buffers.h"
#include "Textures.h"

using DX::ThrowIfFailed;
using Microsoft::WRL::ComPtr;

namespace dxh
{


dxh::Device::Device(IDXGIFactory4* factory)
{
  ComPtr<IDXGIAdapter1> hardwareAdapter;
  DX::GetHardwareAdapter(factory, hardwareAdapter.GetAddressOf(), false);

  ThrowIfFailed(D3D12CreateDevice(
    hardwareAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(device.ReleaseAndGetAddressOf())
  ));
}

void Device::CreateCBV(const dxh::Buffer& buffer, D3D12_CPU_DESCRIPTOR_HANDLE handle)
{
  D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc{};
  cbvDesc.BufferLocation = buffer.GPUVirtualAddress();
  cbvDesc.SizeInBytes = buffer.ByteSize();
  device->CreateConstantBufferView(&cbvDesc, handle);
}

void Device::CreateDSV(const dxh::Texture& texture, D3D12_CPU_DESCRIPTOR_HANDLE handle)
{
  D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
  dsvDesc.Format = texture.Format();
  dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
  dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

  device->CreateDepthStencilView(texture.Resource(), &dsvDesc, handle);
}

}  // namespace dxh