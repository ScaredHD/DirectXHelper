#include "Fence.h"


using DX::ThrowIfFailed;

dxh::Fence::Fence(ID3D12Device* device)
{
  ThrowIfFailed(
      device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(fence_.ReleaseAndGetAddressOf())));
}

void dxh::Fence::WaitForGPUCompletion(ID3D12CommandQueue* cmdQueue)
{
  auto fenceValue = ++nextFenceValue_;
  ThrowIfFailed(cmdQueue->Signal(fence_.Get(), fenceValue));

  if (fence_->GetCompletedValue() < fenceValue) {
    HANDLE event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    ThrowIfFailed(fence_->SetEventOnCompletion(fenceValue, event));
    WaitForSingleObject(event, INFINITE);
  }
}