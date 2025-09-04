
#pragma once

#include "PCH.h"

namespace dxh
{

CD3DX12_CLEAR_VALUE ZeroClearValue(DXGI_FORMAT format);

CD3DX12_CLEAR_VALUE BlackClearValue(DXGI_FORMAT format);

CD3DX12_CLEAR_VALUE RGBAClearValue(DXGI_FORMAT format, float r, float g, float b, float a);

CD3DX12_CLEAR_VALUE DefaultDepthStencilClearValue(DXGI_FORMAT format);

std::string ResourceStateToString(D3D12_RESOURCE_STATES state);

int ComputePaddedSize(int size, int alignment);

struct StateTracker {

  void RecordTransition(D3D12_RESOURCE_STATES newState)
  {
    prevState = state;
    state = newState;
  }

  void UndoTransition() { std::swap(state, prevState); }

  D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON;
  D3D12_RESOURCE_STATES prevState = D3D12_RESOURCE_STATE_COMMON;
};


class TrackedResource
{
public:
  explicit TrackedResource(
    ID3D12Device* device,
    const D3D12_RESOURCE_DESC& desc,
    D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON,
    const D3D12_CLEAR_VALUE* clearValue = nullptr,
    D3D12_HEAP_TYPE heapType = D3D12_HEAP_TYPE_DEFAULT,
    D3D12_HEAP_FLAGS heapFlags = D3D12_HEAP_FLAG_NONE
  )
      : desc{desc},
        heapType{heapType},
        heapFlags{heapFlags},
        clearValue{clearValue}
  {
    auto heapProp = CD3DX12_HEAP_PROPERTIES(heapType, 0, 0);
    DX::ThrowIfFailed(device->CreateCommittedResource(
      &heapProp, heapFlags, &desc, state, clearValue, IID_PPV_ARGS(resource.GetAddressOf())
    ));

    tracker.RecordTransition(state);
  };

  explicit TrackedResource(
    const Microsoft::WRL::ComPtr<ID3D12Resource>& resource,
    D3D12_RESOURCE_STATES currentState,
    const std::string& name = "UnnamedResource"
  )
  {
    this->resource = resource;
    desc = resource->GetDesc();
    D3D12_HEAP_PROPERTIES heapProps;
    resource->GetHeapProperties(&heapProps, &heapFlags);
    heapType = heapProps.Type;
    tracker.RecordTransition(currentState);
    Rename(name);
  }

  virtual ID3D12Resource* Resource() const { return resource.Get(); }

  bool IsValid() const { return resource != nullptr; }

  virtual std::string Name() const { return name; }
  void Rename(const std::string& newName)
  {
    name = newName;
    if (resource) {
      resource->SetName(std::wstring(name.begin(), name.end()).c_str());
    }
  }

  D3D12_GPU_VIRTUAL_ADDRESS GPUVirtualAddress() const { return resource->GetGPUVirtualAddress(); }

  ID3D12Resource** GetAddressOf() { return resource.GetAddressOf(); }

  D3D12_RESOURCE_STATES State() const { return tracker.state; }
  D3D12_RESOURCE_STATES PrevState() const { return tracker.prevState; }

  virtual bool
  ValidateTransition(D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter) const
  {
    return true;
  }

  bool MakeValidatedTransition(ID3D12GraphicsCommandList* cmdList, D3D12_RESOURCE_STATES newState)
  {
    if (tracker.state == newState) {
      return true;  // No transition needed
    }
    if (!ValidateTransition(tracker.state, newState)) {
      return false;
    }
    {
      auto t = CD3DX12_RESOURCE_BARRIER::Transition(resource.Get(), tracker.state, newState);
      cmdList->ResourceBarrier(1, &t);
    }
    tracker.RecordTransition(newState);
    return true;
  }

private:
  Microsoft::WRL::ComPtr<ID3D12Resource> resource;
  D3D12_RESOURCE_DESC desc;
  D3D12_HEAP_TYPE heapType;
  D3D12_HEAP_FLAGS heapFlags;
  const D3D12_CLEAR_VALUE* clearValue;
  StateTracker tracker;
  std::string name = "UnnamedResource";
};


}  // namespace dxh
