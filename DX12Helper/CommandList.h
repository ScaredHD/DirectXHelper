#pragma once

#include "PCH.h"


namespace dxh
{

class GraphicsCommandList
{
public:
  explicit GraphicsCommandList(ID3D12Device* device, ID3D12CommandAllocator* alloc);

  ID3D12GraphicsCommandList* Get() const { return cmdList_.Get(); }

  void Close() const { cmdList_->Close(); }


private:
  Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList_;
};


}  // namespace dxh