#ifndef DXH_DEVICE_H_
#define DXH_DEVICE_H_

#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl/client.h>

namespace dxh
{

class Device
{
public:
  explicit Device(IDXGIFactory4* factory);

  ID3D12Device* Get() const { return device_.Get(); }

private:
  Microsoft::WRL::ComPtr<ID3D12Device> device_;
};

}  // namespace dxh


#endif  // DXH_DEVICE_H_