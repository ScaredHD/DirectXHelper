#pragma once


#include "PCH.h"

namespace dxh
{

class Fence
{
public:
  explicit Fence(ID3D12Device* device);


  void WaitForGPUCompletion(ID3D12CommandQueue* cmdQueue);

private:
  Microsoft::WRL::ComPtr<ID3D12Fence> fence_;
  UINT64 nextFenceValue_ = 0;
};


}  // namespace dxh
