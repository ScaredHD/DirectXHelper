#include "CommandQueue.h"


using DX::ThrowIfFailed;


dxh::CommandQueue::CommandQueue(ID3D12Device* device)
{
  D3D12_COMMAND_QUEUE_DESC desc{};
  desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
  desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
  ThrowIfFailed(
      device->CreateCommandQueue(&desc, IID_PPV_ARGS(cmdQueue_.ReleaseAndGetAddressOf())));
}
