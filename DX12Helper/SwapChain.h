
#pragma once


#include "PCH.h"

namespace dxh
{

template<size_t bufferCount>
class SwapChain
{
public:
  SwapChain(
    ID3D12Device* device,
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

      Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain;
      ThrowIfFailed(factory->CreateSwapChainForHwnd(
        cmdQueue, window, &swapChainDesc, nullptr, nullptr, swapChain.GetAddressOf()
      ));
      ThrowIfFailed(factory->MakeWindowAssociation(window, DXGI_MWA_NO_ALT_ENTER));

      ThrowIfFailed(swapChain.As(&this->swapChain));
    }

    {
      auto desc = CD3DX12_RESOURCE_DESC::Tex2D(
        DXGI_FORMAT_R8G8B8A8_UNORM, viewportWidth, viewportHeight, 1
      );
      desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
      auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
      FLOAT clearColor[] = {0.0f, 0.0f, 0.0f, 1.0f};
      auto clearValue = CD3DX12_CLEAR_VALUE(DXGI_FORMAT_R8G8B8A8_UNORM, clearColor);

      for (int i = 0; i < bufferCount; ++i) {
        ThrowIfFailed(swapChain->GetBuffer(i, IID_PPV_ARGS(&buffers[i])));
        device->CreateCommittedResource(
          &heapProps, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_PRESENT, &clearValue,
          IID_PPV_ARGS(&buffers[i])
        );
      }
    }
  }

  IDXGISwapChain4* Get() const { return swapChain.Get(); }

  ID3D12Resource* Buffer(int i) const { return buffers[i].Get(); }


private:
  Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain;
  std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, bufferCount> buffers;
};

template<size_t bufferCount>
class SwapChainRender
{
public:
  SwapChainRender(
    SwapChain<bufferCount>& swapChain,
    std::array<D3D12_CPU_DESCRIPTOR_HANDLE, bufferCount> bufferRTVs
  )
      : swapChain{swapChain},
        bufferRTVs{bufferRTVs}
  {
  }

  D3D12_CPU_DESCRIPTOR_HANDLE CurrentRTV() { return bufferRTVs[currentIndex]; }

  D3D12_CPU_DESCRIPTOR_HANDLE RTV(int i) { return bufferRTVs[i]; }

  void Swap() { currentIndex = (currentIndex + 1) % bufferCount; }

  void Present() { ThrowIfFailed(swapChain.Get()->Present(1, 0)); }

private:
  SwapChain<bufferCount>& swapChain;
  std::array<D3D12_CPU_DESCRIPTOR_HANDLE, bufferCount> bufferRTVs;

  size_t currentIndex = 0;
};


}  // namespace dxh
