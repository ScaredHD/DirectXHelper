#include "Fence.h"


using DX::ThrowIfFailed;

namespace dxh
{


Fence::Fence(ID3D12Device* device)
{
  ThrowIfFailed(
    device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(fence.ReleaseAndGetAddressOf()))
  );
}

void Fence::SignalToCommandQueue(ID3D12CommandQueue* cmdQueue, uint64_t value)
{
  ThrowIfFailed(cmdQueue->Signal(fence.Get(), value));
}

void Fence::FlushCommandQueue(ID3D12CommandQueue* cmdQueue)
{
  auto fenceValue = ++nextFenceValue;
  SignalToCommandQueue(cmdQueue, fenceValue);

  if (fence->GetCompletedValue() < fenceValue) {
    HANDLE event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    ThrowIfFailed(fence->SetEventOnCompletion(fenceValue, event));
    WaitForSingleObject(event, INFINITE);
  }
}

}  // namespace dxh