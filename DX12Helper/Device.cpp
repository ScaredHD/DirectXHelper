#include "Device.h"

#include "Buffers.h"

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

}  // namespace dxh