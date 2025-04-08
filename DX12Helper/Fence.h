#ifndef DXH_FENCE_H_
#define DXH_FENCE_H_


#include <d3d12.h>
#include <wrl/client.h>

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

#endif  // DXH_FENCE_H_