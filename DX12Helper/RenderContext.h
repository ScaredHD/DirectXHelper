#pragma once

#include "CommandAllocator.h"
#include "CommandList.h"
#include "CommandQueue.h"
#include "DescriptorHeap.h"
#include "Device.h"
#include "Fence.h"
#include "PCH.h"
#include "SwapChain.h"


namespace dxh
{

struct RenderContext {
  explicit RenderContext(IDXGIFactory4* factory, HWND hwnd, int width, int height)
      : device{std::make_unique<Device>(factory)},
        rtvPool{device->Get()},
        dsvPool{device->Get()},
        cbvSrvUavPool{device->Get()}
  {
    cmdQueue = std::make_unique<CommandQueue>(device->Get());
    swapChain = std::make_unique<SwapChain<2>>(factory, cmdQueue->Get(), hwnd, width, height);

    auto rtv0 = rtvPool.Allocate();
    auto rtv1 = rtvPool.Allocate();

    std::array<D3D12_CPU_DESCRIPTOR_HANDLE, 2> rtvHandles = {rtv0, rtv1};
    swapChainManager = std::make_unique<SwapChainManager<2>>(device->Get(), *swapChain, rtvHandles);

    fence = std::make_unique<Fence>(device->Get());
  }

  std::unique_ptr<Device> device;

  RTVPool rtvPool;
  DSVPool dsvPool;
  CbvSrvUavPool cbvSrvUavPool;

  std::unique_ptr<SwapChain<2>> swapChain;
  std::unique_ptr<SwapChainManager<2>> swapChainManager;
  std::unique_ptr<CommandQueue> cmdQueue;
  std::unique_ptr<Fence> fence;

  void FlushCommandQueue() const { fence->FlushCommandQueue(cmdQueue->Get()); }

  void PrepareSwapChainForRender(GraphicsCommandList& cmdList) const
  {
    cmdList.Transition(*swapChainManager->CurrentBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET);

    auto rtv = swapChainManager->CurrentRTV();
    cmdList.SetRenderTargets(1, &rtv);
  }

  void PrepareSwapChainForPresent(GraphicsCommandList& cmdList) const
  {
    cmdList.Transition(*swapChainManager->CurrentBuffer(), D3D12_RESOURCE_STATE_PRESENT);
  }

  void Present() const { swapChainManager->Present(); }

  void CloseAndExecute(GraphicsCommandList& cmdList) const
  {
    cmdList.Close();
    cmdList.Execute(*cmdQueue);
  }
};

}  // namespace dxh