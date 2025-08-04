#pragma once

#include "CommandObjects.h"
#include "DescriptorHeap.h"
#include "Device.h"
#include "PCH.h"
#include "SwapChain.h"


namespace dxh
{

struct RenderContext {




  std::unique_ptr<Device> device;
  std::unique_ptr<CommandQueue> cmdQueue;
  std::unique_ptr<CommandAllocator> cmdAlloc;
  std::unique_ptr<DescriptorHeap> rtvHeap;
  std::unique_ptr<DescriptorHeap> dsvHeap;
  std::unique_ptr<SwapChain<2>> swapChain;
  




};

}  // namespace dxh