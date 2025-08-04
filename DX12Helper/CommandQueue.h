#pragma once

#include "PCH.h"

namespace dxh
{

class CommandQueue
{
public:
  explicit CommandQueue(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT);

  ID3D12CommandQueue* Get() const { return cmdQueue.Get(); }

  void Signal(ID3D12Fence* fence, uint64_t value);

private:
  Microsoft::WRL::ComPtr<ID3D12CommandQueue> cmdQueue;
  D3D12_COMMAND_QUEUE_DESC desc;
};

}  // namespace dxh