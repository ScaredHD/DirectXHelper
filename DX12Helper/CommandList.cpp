#include "CommandList.h"

#include "CommandAllocator.h"
#include "CommandQueue.h"

using DX::ThrowIfFailed;

namespace dxh
{

GraphicsCommandList::GraphicsCommandList(ID3D12Device* device, ID3D12CommandAllocator* alloc)
{
  ThrowIfFailed(device->CreateCommandList(
    0, D3D12_COMMAND_LIST_TYPE_DIRECT, alloc, nullptr,
    IID_PPV_ARGS(cmdList.ReleaseAndGetAddressOf())
  ));
}

void GraphicsCommandList::Reset(CommandAllocator& alloc)
{
  ThrowIfFailed(cmdList->Reset(alloc.Get(), nullptr));
}

void GraphicsCommandList::Reset(ID3D12CommandAllocator* alloc)
{
  ThrowIfFailed(cmdList->Reset(alloc, nullptr));
}

void GraphicsCommandList::Execute(ID3D12CommandQueue* cmdQueue) const
{
  ID3D12CommandList* const cmdLists[] = {cmdList.Get()};
  cmdQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);
}

void GraphicsCommandList::Execute(CommandQueue& cmdQueue) const
{
  Execute(cmdQueue.Get());
}


}  // namespace dxh