#pragma once

#include "PCH.h"

namespace dxh
{

class CommandQueue
{
public:
  explicit CommandQueue(ID3D12Device* device);

  ID3D12CommandQueue* Get() const { return cmdQueue.Get(); }

  void Signal(ID3D12Fence* fence, uint64_t value);

private:
  Microsoft::WRL::ComPtr<ID3D12CommandQueue> cmdQueue;
};

}  // namespace dxh