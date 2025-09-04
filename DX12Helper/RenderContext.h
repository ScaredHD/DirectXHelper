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
    std::array<D3D12_CPU_DESCRIPTOR_HANDLE, 2> dsvHandles = {
      dsvPool.Allocate(), dsvPool.Allocate()
    };
    swapChainManager = std::make_unique<SwapChainManager<2>>(
      device->Get(), *swapChain, rtvHandles, dsvHandles
    );

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

  void
  PrepareSwapChainForRender(GraphicsCommandList& cmdList, D3D12_CPU_DESCRIPTOR_HANDLE dsv) const
  {
    cmdList.Transition(*swapChainManager->CurrentBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET);
    cmdList.Transition(*swapChainManager->CurrentDepthBuffer(), D3D12_RESOURCE_STATE_DEPTH_WRITE);

    auto rtv = swapChainManager->CurrentRTV();
    cmdList.SetRenderTargets(1, &rtv, &dsv);
    cmdList.ClearDSV(dsv, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0);
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

  void ClearBackBuffer(GraphicsCommandList& cmdList, const DirectX::XMFLOAT4& color) const
  {
    auto rtv = swapChainManager->CurrentRTV();
    cmdList.ClearRTV(rtv, {color.x, color.y, color.z, color.w});
  }
};

}  // namespace dxh