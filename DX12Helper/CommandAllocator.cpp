#include "CommandAllocator.h"



using DX::ThrowIfFailed;

namespace dxh
{

CommandAllocator::CommandAllocator(ID3D12Device* device)
{
	ThrowIfFailed(device->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(cmdAlloc.ReleaseAndGetAddressOf())
	));
}

void CommandAllocator::Reset()
{
	ThrowIfFailed(cmdAlloc->Reset());
}

}  // namespace dxh
