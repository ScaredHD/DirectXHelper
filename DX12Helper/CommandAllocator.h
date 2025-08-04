#pragma once

#include "PCH.h"

namespace dxh
{

class CommandAllocator
{
public:
  explicit CommandAllocator(ID3D12Device* device);

  void Reset();

  ID3D12CommandAllocator* Get() const { return cmdAlloc.Get(); }

private:
  Microsoft::WRL::ComPtr<ID3D12CommandAllocator> cmdAlloc;
};

}  // namespace dxh
