#pragma once

#include "PCH.h"
#include "RootSignature.h"
#include "SwapChain.h"


namespace dxh
{

class TrackedResource;

class GraphicsCommandList
{
public:
  explicit GraphicsCommandList(ID3D12Device* device, ID3D12CommandAllocator* alloc);

  ID3D12GraphicsCommandList* Get() const { return cmdList.Get(); }

  void Reset(class CommandAllocator& alloc);

  void Reset(ID3D12CommandAllocator* alloc);

  void Close() const { cmdList->Close(); }

  void Execute(ID3D12CommandQueue* cmdQueue) const;

  void Execute(class CommandQueue& cmdQueue) const;

  void Transition(
    ID3D12Resource* resource,
    D3D12_RESOURCE_STATES stateBefore,
    D3D12_RESOURCE_STATES stateAfter
  ) const
  {
    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(resource, stateBefore, stateAfter);
    cmdList->ResourceBarrier(1, &barrier);
  }

  void Transition(TrackedResource& resource, D3D12_RESOURCE_STATES targetState);

  void SetRootSignature(const RootSignature& rootSignature)
  {
    cmdList->SetGraphicsRootSignature(rootSignature.GetRootSignature());
  }

  void SetRootCBV(UINT rootParameterIndex, ID3D12Resource* resource)
  {
    cmdList->SetGraphicsRootConstantBufferView(
      rootParameterIndex, resource->GetGPUVirtualAddress()
    );
  }

  void SetViewport(const SwapChain<2>& swapChain) const
  {
    auto viewport = dxh::MakeViewport(swapChain);
    cmdList->RSSetViewports(1, &viewport);
  }

  void SetScissorRect(const SwapChain<2>& swapChain) const
  {
    auto scissorRect = dxh::MakeScissorRect(swapChain);
    cmdList->RSSetScissorRects(1, &scissorRect);
  }

  void SetRenderTargets(UINT count, D3D12_CPU_DESCRIPTOR_HANDLE renderTargets[])
  {
    cmdList->OMSetRenderTargets(count, renderTargets, FALSE, nullptr);
  }

  void ClearRTV(D3D12_CPU_DESCRIPTOR_HANDLE rtv, std::array<float, 4> color) const
  {
    cmdList->ClearRenderTargetView(rtv, color.data(), 0, nullptr);
  }


private:
  Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList;
};


}  // namespace dxh