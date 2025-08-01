#pragma once

#include "PCH.h"

namespace dxh
{

class CommandQueue
{
public:
  explicit CommandQueue(ID3D12Device* device);

  ID3D12CommandQueue* Get() const { return cmdQueue_.Get(); }

private:
  Microsoft::WRL::ComPtr<ID3D12CommandQueue> cmdQueue_;
};

}  // namespace dxh