#pragma once

#include "PCH.h"


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

private:
  Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList;
};


}  // namespace dxh