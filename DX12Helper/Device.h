
#pragma once

#include "PCH.h"



namespace dxh
{

class Buffer;
class Texture;

class Device
{
public:
  explicit Device(IDXGIFactory4* factory);

  ID3D12Device* Get() const { return device.Get(); }
  Microsoft::WRL::ComPtr<ID3D12Device> GetComPtr() const { return device; }

  void CreateCBV(const dxh::Buffer& buffer, D3D12_CPU_DESCRIPTOR_HANDLE handle);
  void CreateSRV(const dxh::Texture& texture, D3D12_CPU_DESCRIPTOR_HANDLE handle);
  void CreateUAV(const dxh::Texture& texture, D3D12_CPU_DESCRIPTOR_HANDLE handle);
  

private:
  Microsoft::WRL::ComPtr<ID3D12Device> device;
};

}  // namespace dxh
