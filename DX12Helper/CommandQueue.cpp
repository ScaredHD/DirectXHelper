#include "CommandQueue.h"


using DX::ThrowIfFailed;


dxh::CommandQueue::CommandQueue(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE type)
{
  D3D12_COMMAND_QUEUE_DESC desc{};
  desc.Type = type;
  desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
  ThrowIfFailed(device->CreateCommandQueue(&desc, IID_PPV_ARGS(cmdQueue.ReleaseAndGetAddressOf())));
  this->desc = desc;
}

void dxh::CommandQueue::Signal(ID3D12Fence* fence, uint64_t value)
{
  DX::ThrowIfFailed(cmdQueue->Signal(fence, value));
}
