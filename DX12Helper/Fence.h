#pragma once


#include "PCH.h"

namespace dxh
{

class Fence
{
public:
  explicit Fence(ID3D12Device* device);

  void SignalToCommandQueue(ID3D12CommandQueue* cmdQueue, uint64_t value);

  UINT64 GetCompletedValue() const { return fence->GetCompletedValue(); }

  void FlushCommandQueue(ID3D12CommandQueue* cmdQueue);

private:
  Microsoft::WRL::ComPtr<ID3D12Fence> fence;
  UINT64 nextFenceValue = 0;
};


}  // namespace dxh
