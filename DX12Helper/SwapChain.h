#ifndef DXH_SWAPCHAIN_H_
#define DXH_SWAPCHAIN_H_

#include <d3d12.h>
#include <dxgi1_4.h>
#include <functional>
#include <vector>
#include <wrl/client.h>

#include "Resources.h"

namespace dxh
{
class SwapChain
{
public:
  /// @param bufferRTVAlloc For i-th buffer in swap chain, return handle to descriptor heap where
  /// this RTV should be created
  explicit SwapChain(
    ID3D12Device* device,
    IDXGIFactory4* factory,
    ID3D12CommandQueue* cmdQueue,
    HWND window,
    unsigned int bufferCount,
    int viewportWidth,
    int viewportHeight,
    std::function<D3D12_CPU_DESCRIPTOR_HANDLE(int)> bufferRTVAlloc
  );

  IDXGISwapChain* Get() const { return swapChain_.Get(); }

  void Swap() { currentBufferIndex = (currentBufferIndex + 1) % bufferCount_; }

  RawResource* Buffer(int i) const { return buffers_[i].get(); }

  RawResource* CurrentBuffer() const { return buffers_[currentBufferIndex].get(); }

  D3D12_CPU_DESCRIPTOR_HANDLE CPURtv(int i) { return swapChainRTVs_[i]; }

  D3D12_CPU_DESCRIPTOR_HANDLE CurrentRTV() const { return swapChainRTVs_[currentBufferIndex]; }

  unsigned int BufferCount() const { return bufferCount_; }

  unsigned int CurrentBufferIndex() const { return currentBufferIndex; }

  void Present() const;


private:
  Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain_;

  std::vector<std::unique_ptr<RawResource>> buffers_;

  std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> swapChainRTVs_;

  int currentBufferIndex = 0;

  unsigned int bufferCount_ = 0;
};


DrawCommands ClearBackBuffer(const SwapChain* swapChain, float r, float g, float b, float a);

// DrawCommands CopyTexture2DToSwapChain(
//   SwapChain* dst,
//   UINT dstX,
//   UINT dstY,
//   RawResource* src,
//   UINT srcXMin,
//   UINT srcYMin,
//   UINT srcXMax,
//   UINT srcYMax
// );

// DrawCommands Transition(SwapChain* swapChain, D3D12_RESOURCE_STATES toState);

}  // namespace dxh

#endif  // DXH_SWAPCHAIN_H_