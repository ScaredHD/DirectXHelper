#pragma once

#include <array>

#include "PCH.h"
#include "Resources.h"

using DX::ThrowIfFailed;

namespace dxh
{

template<size_t bufferCount>
class SwapChain
{
public:
  SwapChain(
    IDXGIFactory4* factory,
    ID3D12CommandQueue* cmdQueue,
    HWND window,
    int viewportWidth,
    int viewportHeight
  )
  {
    {
      DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
      swapChainDesc.BufferCount = bufferCount;
      swapChainDesc.Width = viewportWidth;
      swapChainDesc.Height = viewportHeight;
      swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
      swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
      swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
      swapChainDesc.SampleDesc.Count = 1;
      desc = swapChainDesc;

      Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain;
      ThrowIfFailed(factory->CreateSwapChainForHwnd(
        cmdQueue, window, &swapChainDesc, nullptr, nullptr, swapChain.GetAddressOf()
      ));
      ThrowIfFailed(factory->MakeWindowAssociation(window, DXGI_MWA_NO_ALT_ENTER));

      ThrowIfFailed(swapChain.As(&this->swapChain));
    }

    for (int i = 0; i < bufferCount; ++i) {
      ThrowIfFailed(swapChain->GetBuffer(i, IID_PPV_ARGS(&buffers[i])));
    }
  }

  UINT CurrentBackBufferIndex() const { return swapChain->GetCurrentBackBufferIndex(); }

  IDXGISwapChain4* Get() const { return swapChain.Get(); }

  ID3D12Resource* Buffer(int i) const { return buffers[i].Get(); }

  DXGI_SWAP_CHAIN_DESC1 Desc() const { return desc; }

  UINT Width() const { return desc.Width; }

  UINT Height() const { return desc.Height; }

  DXGI_FORMAT Format() const { return desc.Format; }

private:
  Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain;
  DXGI_SWAP_CHAIN_DESC1 desc;
  std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, bufferCount> buffers;
};

template<size_t bufferCount>
class SwapChainManager
{
public:
  SwapChainManager(
    ID3D12Device* device,
    SwapChain<bufferCount>& swapChain,
    std::array<D3D12_CPU_DESCRIPTOR_HANDLE, bufferCount> bufferRTVs
  )
      : swapChain{swapChain},
        buffers{},
        bufferRTVs{bufferRTVs}
  {
    for (size_t i = 0; i < bufferCount; ++i) {
      D3D12_RENDER_TARGET_VIEW_DESC desc{};
      desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
      desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
      desc.Texture2D.MipSlice = 0;
      desc.Texture2D.PlaneSlice = 0;
      ID3D12Resource* buffer = swapChain.Buffer(i);
      buffers[i] = std::make_unique<dxh::TrackedResource>(buffer, std::string("SwapChainBuffer") + std::to_string(i));
      if (buffer) {
        device->CreateRenderTargetView(buffer, &desc, bufferRTVs[i]);
      }
    }
  }

  D3D12_CPU_DESCRIPTOR_HANDLE CurrentRTV()
  {
    return bufferRTVs[swapChain.CurrentBackBufferIndex()];
  }

  dxh::TrackedResource* CurrentBuffer()
  {
    return buffers[swapChain.CurrentBackBufferIndex()].get();
  }

  void Present() { ThrowIfFailed(swapChain.Get()->Present(1, 0)); }

private:
  SwapChain<bufferCount>& swapChain;
  std::array<std::unique_ptr<dxh::TrackedResource>, bufferCount> buffers;
  std::array<D3D12_CPU_DESCRIPTOR_HANDLE, bufferCount> bufferRTVs;
};

template<size_t n>
CD3DX12_VIEWPORT MakeViewport(const SwapChain<n>& swapChain)
{
  return CD3DX12_VIEWPORT{
    0.f, 0.f, static_cast<float>(swapChain.Width()), static_cast<float>(swapChain.Height())
  };
}

template<size_t n>
D3D12_RECT MakeScissorRect(const SwapChain<n>& swapChain)
{
  return {0, 0, static_cast<LONG>(swapChain.Width()), static_cast<LONG>(swapChain.Height())};
}


}  // namespace dxh
