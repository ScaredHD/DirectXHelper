#pragma once

#include "CommandAllocator.h"
#include "CommandQueue.h"
#include "Device.h"
#include "Fence.h"
#include "PCH.h"
#include "SwapChain.h"



namespace dxh
{

struct RenderContext {
  explicit RenderContext(IDXGIFactory4* factory, HWND window, int width, int height)
      : device{factory},
        cmdQueue{device.Get()},
        swapChain{factory, cmdQueue.Get(), window, width, height},
        cmdAlloc{device.Get()},
        fence{device.Get()}
  {
  }

  explicit RenderContext(
    std::unique_ptr<Device> device,
    std::unique_ptr<CommandQueue> cmdQueue,
    std::unique_ptr<SwapChain<2>> swapChain,
    std::unique_ptr<CommandAllocator> cmdAlloc
  )
      : device{std::move(*device)},
        cmdQueue{std::move(*cmdQueue)},
        swapChain{std::move(*swapChain)},
        cmdAlloc{std::move(*cmdAlloc)},
        fence{device->Get()}
  {
  }

  Device device;
  CommandQueue cmdQueue;
  SwapChain<2> swapChain;
  CommandAllocator cmdAlloc;
  Fence fence;
};


}  // namespace dxh