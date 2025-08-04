
#pragma once

#include <d3d12.h>
#include <d3dx12.h>
#include <stdexcept>



namespace DX
{
// Helper class for COM exceptions
class com_exception : public std::exception
{
public:
  com_exception(HRESULT hr) : result(hr) {}

  const char* what() const noexcept override
  {
    static char s_str[64] = {};
    sprintf_s(s_str, "Failure with HRESULT of %08X", static_cast<unsigned int>(result));
    return s_str;
  }

private:
  HRESULT result;
};

// Helper utility converts D3D API failures into exceptions.
inline void ThrowIfFailed(HRESULT hr)
{
  if (FAILED(hr)) {
    throw com_exception(hr);
  }
}

inline void GetHardwareAdapter(
  IDXGIFactory1* pFactory,
  IDXGIAdapter1** ppAdapter,
  bool requestHighPerformanceAdapter
)
{
  *ppAdapter = nullptr;

  Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;

  Microsoft::WRL::ComPtr<IDXGIFactory6> factory6;
  if (SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&factory6)))) {
    for (UINT adapterIndex = 0; SUCCEEDED(factory6->EnumAdapterByGpuPreference(
           adapterIndex,
           requestHighPerformanceAdapter == true ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE
                                                 : DXGI_GPU_PREFERENCE_UNSPECIFIED,
           IID_PPV_ARGS(&adapter)
         ));
         ++adapterIndex) {
      DXGI_ADAPTER_DESC1 desc;
      adapter->GetDesc1(&desc);

      if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
        // Don't select the Basic Render Driver adapter.
        // If you want a software adapter, pass in "/warp" on the command line.
        continue;
      }

      // Check to see whether the adapter supports Direct3D 12, but don't create the
      // actual device yet.
      if (SUCCEEDED(D3D12CreateDevice(
            adapter.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr
          ))) {
        break;
      }
    }
  }

  if (adapter.Get() == nullptr) {
    for (UINT adapterIndex = 0; SUCCEEDED(pFactory->EnumAdapters1(adapterIndex, &adapter));
         ++adapterIndex) {
      DXGI_ADAPTER_DESC1 desc;
      adapter->GetDesc1(&desc);

      if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
        // Don't select the Basic Render Driver adapter.
        // If you want a software adapter, pass in "/warp" on the command line.
        continue;
      }

      // Check to see whether the adapter supports Direct3D 12, but don't create the
      // actual device yet.
      if (SUCCEEDED(D3D12CreateDevice(
            adapter.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr
          ))) {
        break;
      }
    }
  }

  *ppAdapter = adapter.Detach();
}
}  // namespace DX


namespace dxh
{

float RandomFloat(float min, float max);

CD3DX12_VIEWPORT MakeViewport(POINT min, POINT max);

D3D12_RECT MakeScissorRect(POINT min, POINT max);


}  // namespace dxh
