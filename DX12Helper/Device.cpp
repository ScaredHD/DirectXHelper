#include "Device.h"


using DX::ThrowIfFailed;
using Microsoft::WRL::ComPtr;


dxh::Device::Device(IDXGIFactory4* factory)
{
  ComPtr<IDXGIAdapter1> hardwareAdapter;
  DX::GetHardwareAdapter(factory, hardwareAdapter.GetAddressOf(), false);

  ThrowIfFailed(D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_11_0,
                                  IID_PPV_ARGS(device_.ReleaseAndGetAddressOf())));
}