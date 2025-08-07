#include "CommandObjects.h"


using DX::ThrowIfFailed;

namespace dxh
{

void ResetCommandList(ID3D12GraphicsCommandList* cmdList, ID3D12CommandAllocator* cmdAlloc)
{
  ThrowIfFailed(cmdList->Reset(cmdAlloc, nullptr));
}

void ResetCommandList(GraphicsCommandList& cmdList, CommandAllocator& cmdAlloc)
{
  cmdList.Reset(cmdAlloc);
}

void ResetCommandAllocator(ID3D12CommandAllocator* cmdAlloc)
{
  ThrowIfFailed(cmdAlloc->Reset());
}

void ResetCommandAllocator(CommandAllocator& cmdAlloc)
{
  cmdAlloc.Reset();
}

void ExecuteCommandList(ID3D12CommandList* cmdList, ID3D12CommandQueue* cmdQueue)
{
  ID3D12CommandList* const cmdLists[] = {cmdList};
  cmdQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);
}

void ExecuteCommandList(GraphicsCommandList& cmdList, CommandQueue& cmdQueue)
{
  ExecuteCommandList(cmdList.Get(), cmdQueue.Get());
}


}  // namespace dxh