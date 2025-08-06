#include "CommandObjects.h"


using DX::ThrowIfFailed;

namespace dxh
{

void ResetCommandList(ID3D12GraphicsCommandList* cmdList, ID3D12CommandAllocator* cmdAlloc)
{
  ThrowIfFailed(cmdList->Reset(cmdAlloc, nullptr));
}

void ResetCommandList(const GraphicsCommandList& cmdList, const CommandAllocator& cmdAlloc)
{
  ResetCommandList(cmdList.Get(), cmdAlloc.Get());
}


void ResetCommandAllocator(ID3D12CommandAllocator* cmdAlloc)
{
  ThrowIfFailed(cmdAlloc->Reset());
}

void ResetCommandAllocator(const CommandAllocator& cmdAlloc)
{
  ResetCommandAllocator(cmdAlloc.Get());
}


void ExecuteCommandList(ID3D12CommandList* cmdList, ID3D12CommandQueue* cmdQueue)
{
  ID3D12CommandList* const cmdLists[] = {cmdList};
  cmdQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);
}

void ExecuteCommandList(const GraphicsCommandList& cmdList, const CommandQueue& cmdQueue)
{
  ExecuteCommandList(cmdList.Get(), cmdQueue.Get());
}


}  // namespace dxh