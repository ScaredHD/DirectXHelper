
#pragma once

#include "PCH.h"

namespace dxh
{

class Device
{
public:
  explicit Device(IDXGIFactory4* factory);

  ID3D12Device* Get() const { return device_.Get(); }

private:
  Microsoft::WRL::ComPtr<ID3D12Device> device_;
};

}  // namespace dxh


