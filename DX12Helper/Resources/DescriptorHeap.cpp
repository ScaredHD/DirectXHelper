#include "DescriptorHeap.h"


namespace dxh
{


DescriptorHeap* DescriptorHeapPool::RequestHeap()
{
  if (currentHeap) {
    return currentHeap.get();
  }

  if (!retiredHeaps.empty()) {
    RetiredHeap& h = retiredHeaps.front();
    // TODO:
    uint64_t completedFence;
    ;
    if (completedFence >= h.fenceValue) {
      availableHeaps.push(std::move(h.heap));
      retiredHeaps.pop();
    }
  }

  if (!availableHeaps.empty()) {
    currentHeap = std::move(availableHeaps.front());
    availableHeaps.pop();
  } else {
    currentHeap = std::make_unique<DescriptorHeap>(
      device.Get(),
      D3D12_DESCRIPTOR_HEAP_DESC{type, descriptorsPerHeap, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, 0}
    );
  }
  nextAvailableIndex = 0;
  return currentHeap.get();
}
DescriptorHeap* DescriptorHeapPool::CurrentHeap()
{
  if (!currentHeap) {
    return RequestHeap();
  }
  return currentHeap.get();
}
void DescriptorHeapPool::RetireHeap(uint64_t fenceValue)
{
  if (!currentHeap) {
    return;
  }

  RetiredHeap retired;
  retired.fenceValue = fenceValue;
  retired.heap = std::move(currentHeap);
  retiredHeaps.push(std::move(retired));
  currentHeap.reset();
}
}  // namespace dxh