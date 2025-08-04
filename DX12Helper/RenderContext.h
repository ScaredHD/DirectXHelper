#pragma once

#include "CommandAllocator.h"
#include "CommandQueue.h"
#include "Device.h"
#include "Fence.h"

namespace dxh
{

struct RenderContext {
  explicit RenderContext() = default;

  std::unique_ptr<Device> device;
  std::unique_ptr<CommandQueue> cmdQueue;
  std::unique_ptr<CommandAllocator> cmdAllocator;
  std::unique_ptr<Fence> fence;
};

}  // namespace dxh
