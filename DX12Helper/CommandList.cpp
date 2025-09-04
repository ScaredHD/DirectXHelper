#include "CommandList.h"

#include <sstream>
#include <string>

#include "CommandAllocator.h"
#include "CommandQueue.h"
#include "GeometryRender.h"
#include "Resources.h"
#include "RootSignature.h"
#include "SwapChain.h"


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

void GraphicsCommandList::Transition(TrackedResource& resource, D3D12_RESOURCE_STATES targetState)
{
  if (!resource.MakeValidatedTransition(cmdList.Get(), targetState)) {
    std::stringstream err;
    err << "Failed to transition resource '" << resource.Name() << "' from "
        << ResourceStateToString(resource.State()) << " to " << ResourceStateToString(targetState);
    throw std::runtime_error(err.str());
  }
}

ID3D12GraphicsCommandList* GraphicsCommandList::Get() const
{
  return cmdList.Get();
}

void GraphicsCommandList::Close() const
{
  cmdList->Close();
}

void GraphicsCommandList::Transition(
  ID3D12Resource* resource,
  D3D12_RESOURCE_STATES stateBefore,
  D3D12_RESOURCE_STATES stateAfter
) const
{
  auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(resource, stateBefore, stateAfter);
  cmdList->ResourceBarrier(1, &barrier);
}

void GraphicsCommandList::SetRootSignature(const RootSignature& rootSignature)
{
  cmdList->SetGraphicsRootSignature(rootSignature.GetRootSignature());
}

void GraphicsCommandList::SetPipelineState(ID3D12PipelineState* pso)
{
  cmdList->SetPipelineState(pso);
}

void GraphicsCommandList::SetRootCBV(UINT rootParameterIndex, ID3D12Resource* resource)
{
  cmdList->SetGraphicsRootConstantBufferView(rootParameterIndex, resource->GetGPUVirtualAddress());
}

void GraphicsCommandList::SetViewport(const SwapChain<2>& swapChain) const
{
  auto viewport = dxh::MakeViewport(swapChain);
  cmdList->RSSetViewports(1, &viewport);
}

void GraphicsCommandList::SetScissorRect(const SwapChain<2>& swapChain) const
{
  auto scissorRect = dxh::MakeScissorRect(swapChain);
  cmdList->RSSetScissorRects(1, &scissorRect);
}

void GraphicsCommandList::SetRenderTargets(
  UINT count,
  D3D12_CPU_DESCRIPTOR_HANDLE renderTargets[],
  D3D12_CPU_DESCRIPTOR_HANDLE dsv[]
)
{
  cmdList->OMSetRenderTargets(count, renderTargets, FALSE, dsv);
}

void GraphicsCommandList::ClearRTV(
  D3D12_CPU_DESCRIPTOR_HANDLE rtv,
  std::array<float, 4> color
) const
{
  cmdList->ClearRenderTargetView(rtv, color.data(), 0, nullptr);
}

void GraphicsCommandList::ClearDSV(
  D3D12_CPU_DESCRIPTOR_HANDLE dsv,
  D3D12_CLEAR_FLAGS flags,
  float depth,
  UINT8 stencil
) const
{
  cmdList->ClearDepthStencilView(dsv, flags, depth, stencil, 0, nullptr);
}

void GraphicsCommandList::SetVBV(D3D12_VERTEX_BUFFER_VIEW vbv)
{
  cmdList->IASetVertexBuffers(0, 1, &vbv);
}

void GraphicsCommandList::SetIBV(D3D12_INDEX_BUFFER_VIEW ibv)
{
  cmdList->IASetIndexBuffer(&ibv);
}

}  // namespace dxh