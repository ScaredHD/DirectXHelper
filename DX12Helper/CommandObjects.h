#pragma once

#include "CommandAllocator.h"
#include "CommandList.h"
#include "CommandQueue.h"

namespace dxh
{

void ResetCommandList(ID3D12GraphicsCommandList* cmdList, ID3D12CommandAllocator* cmdAlloc);

void ResetCommandList(const GraphicsCommandList& cmdList, const CommandAllocator& cmdAlloc);

void ResetCommandAllocator(ID3D12CommandAllocator* cmdAlloc);

void ResetCommandAllocator(const CommandAllocator& cmdAlloc);

void ExecuteCommandList(ID3D12CommandList* cmdList, ID3D12CommandQueue* cmdQueue);

void ExecuteQueuedCommands(
  GraphicsCommandList& cmdList,
  const CommandQueue& cmdQueue
);


}  // namespace dxh