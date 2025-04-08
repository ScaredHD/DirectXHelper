#include "CommandList.h"

#include "D3DUtils.h"

using DX::ThrowIfFailed;

namespace dxh
{
  
GraphicsCommandList::GraphicsCommandList(ID3D12Device* device, ID3D12CommandAllocator* alloc)
{
  ThrowIfFailed(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, alloc, nullptr,
                                          IID_PPV_ARGS(cmdList_.ReleaseAndGetAddressOf())));
}


} // namespace dxh