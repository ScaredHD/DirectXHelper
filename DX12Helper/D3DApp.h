#ifndef D3DAPP_H_
#define D3DAPP_H_

#include <memory>
#include <wrl/client.h>

#include "CommandObjects.h"
#include "DescriptorHeap.h"
#include "Device.h"
#include "Fence.h"
#include "RenderGeometry.h"
#include "Resources.h"
#include "Shader.h"
#include "SwapChain.h"

class D3DDefaultApp
{
public:
  explicit D3DDefaultApp(HWND window);

  void Update();

  void Render();

  bool IsRunning() const { return true; }

  virtual void OnRender();

  virtual void OnUpdate() {}

  float frameTimeMs = 0.f;
  float totalTimeElapsedMs = 0.f;

protected:
  void InitializeD3D(HWND window);

  void WaitForGPUCompletion() const { fence_->WaitForGPUCompletion(cmdQueue_->Get()); }

  CD3DX12_VIEWPORT viewport_{};
  CD3DX12_RECT scissorRect_{};

  std::unique_ptr<dxh::Device> device_;

  std::unique_ptr<dxh::SwapChain> swapChain_;


  std::unique_ptr<dxh::CommandQueue> cmdQueue_;
  std::unique_ptr<dxh::CommandAllocator> cmdAllocator_;
  std::unique_ptr<dxh::GraphicsCommandList> cmdList_;

  std::shared_ptr<dxh::DescriptorHeap> rtvHeap_;
  std::shared_ptr<dxh::DescriptorHeap> dsvHeap_;
  std::shared_ptr<dxh::DescriptorHeap> cbvSrvUavHeap_;

  std::unique_ptr<dxh::DepthStencilBuffer> depthBuffer_;

  std::unique_ptr<dxh::Fence> fence_;

  std::unique_ptr<dxh::Shader> vs_;
  std::unique_ptr<dxh::Shader> ps_;

  Microsoft::WRL::ComPtr<ID3D12PipelineState> pso_;
  Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;

  std::unique_ptr<dxh::DefaultVertexBuffer> vBuffer_;
  std::unique_ptr<dxh::DefaultIndexBuffer> iBuffer_;

  std::unique_ptr<dxh::UploadHeapArray<dxh::DefaultVertex>> uploadVBuffer_;
  D3D12_VERTEX_BUFFER_VIEW vbv_;
};


#endif  // D3DAPP_H_