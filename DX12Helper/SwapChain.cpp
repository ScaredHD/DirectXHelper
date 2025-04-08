#include "SwapChain.h"

#include "D3DUtils.h"

using DX::ThrowIfFailed;
using Microsoft::WRL::ComPtr;

namespace dxh
{


SwapChain::SwapChain(
  ID3D12Device* device,
  IDXGIFactory4* factory,
  ID3D12CommandQueue* cmdQueue,
  HWND window,
  unsigned int bufferCount,
  int viewportWidth,
  int viewportHeight,
  std::function<D3D12_CPU_DESCRIPTOR_HANDLE(int)> bufferRTVAlloc
)
    : bufferCount_{bufferCount}
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

    ComPtr<IDXGISwapChain1> swapChain;
    ThrowIfFailed(factory->CreateSwapChainForHwnd(
      cmdQueue, window, &swapChainDesc, nullptr, nullptr, swapChain.GetAddressOf()
    ));
    ThrowIfFailed(factory->MakeWindowAssociation(window, DXGI_MWA_NO_ALT_ENTER));

    ThrowIfFailed(swapChain.As(&swapChain_));
  }

  // Allocate swap chain buffers
  buffers_.resize(bufferCount);
  for (int i = 0; i < bufferCount; ++i) {
    buffers_[i] = std::make_unique<RawResource>();
    buffers_[i]->SetName("swap chain buffer" + std::to_string(i));
    // buffers_[i]->SetDebug(true);
    ThrowIfFailed(swapChain_.Get()->GetBuffer(i, IID_PPV_ARGS(buffers_[i]->GetAddressOf())));
  }

  // Create swap chain buffer RTVs
  swapChainRTVs_.resize(bufferCount);
  for (int i = 0; i < bufferCount; ++i) {
    auto rtv = bufferRTVAlloc(i);
    device->CreateRenderTargetView(buffers_[i]->Resource(), nullptr, rtv);
    swapChainRTVs_[i] = rtv;
  }
}

void SwapChain::Present() const
{
  ThrowIfFailed(swapChain_->Present(0, 0));
}

DrawCommands ClearBackBuffer(const dxh::SwapChain* swapChain, float r, float g, float b, float a)
{
  return [=](ID3D12GraphicsCommandList* cmdList) {
    float clearColor[] = {r, g, b, a};
    cmdList->ClearRenderTargetView(swapChain->CurrentRTV(), clearColor, 0, nullptr);
  };
}

// DrawCommands Transition(SwapChain* swapChain, D3D12_RESOURCE_STATES toState)
// {
//   auto stateBefore = swapChain->currentState;
//   swapChain->currentState = toState;
//   return Transition(swapChain->CurrentBuffer(), stateBefore, toState);
// }

// DrawCommands CopyTexture2DToSwapChain(
//   SwapChain* dst,
//   UINT dstX,
//   UINT dstY,
//   RawResource* src,
//   UINT srcXMin,
//   UINT srcYMin,
//   UINT srcXMax,
//   UINT srcYMax
// )
// {
//   auto dstStateOld = dst->currentState;
//   auto srcStateOld = src->CurrentState();
//   return MergeCommands(
//     dxh::Transition(dst->CurrentBuffer(), dstStateOld, D3D12_RESOURCE_STATE_COPY_DEST),
//     dxh::Transition(src->Resource(), srcStateOld, D3D12_RESOURCE_STATE_COPY_SOURCE),
//     dxh::CopyTexture(
//       dst->CurrentBuffer(), dstX, dstY, 0, src->Resource(), srcXMin, srcYMin, 0, srcXMax,
//       srcYMax, 1
//     ),
//     dxh::Transition(src->Resource(), D3D12_RESOURCE_STATE_COPY_SOURCE, srcStateOld),
//     dxh::Transition(dst->CurrentBuffer(), D3D12_RESOURCE_STATE_COPY_DEST, dstStateOld)
//   );
// }

}  // namespace dxh