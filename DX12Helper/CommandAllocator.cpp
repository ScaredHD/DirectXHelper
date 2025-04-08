#include "CommandAllocator.h"

#include "D3DUtils.h"

using DX::ThrowIfFailed;

dxh::CommandAllocator::CommandAllocator(ID3D12Device* device)
{
  ThrowIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                               IID_PPV_ARGS(cmdAlloc_.ReleaseAndGetAddressOf())));
}