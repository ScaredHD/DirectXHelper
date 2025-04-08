#ifndef DXH_COMMANDQUEUE_H_
#define DXH_COMMANDQUEUE_H_


#include <d3d12.h>
#include <wrl/client.h>

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


#endif  // DXH_COMMANDQUEUE_H_