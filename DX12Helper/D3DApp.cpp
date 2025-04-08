#include "D3DApp.h"

#include <cassert>
#include <DirectXMath.h>
#include <iostream>
#include <stdexcept>

#include "AutoTimer.h"
#include "D3DUtils.h"
#include "GraphicsToolkit/random/sample.h"
#include "RenderGeometry.h"
#include "Shapes.h"

using DX::ThrowIfFailed;
using Microsoft::WRL::ComPtr;

using namespace DirectX;


D3DDefaultApp::D3DDefaultApp(HWND window)
{
  assert(window != NULL);

  if (RECT rect; GetClientRect(window, &rect)) {
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;
    viewport_.Width = width;
    viewport_.Height = height;

    scissorRect_ = CD3DX12_RECT{0, 0, width, height};
  } else {
    throw std::runtime_error{"failed to read client rect"};
  }

  InitializeD3D(window);
}


void D3DDefaultApp::InitializeD3D(HWND window)
{
  UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG)
  if (ComPtr<ID3D12Debug> debugController;
      SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(debugController.GetAddressOf())))) {
    debugController->EnableDebugLayer();
    dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
  }
#endif

  ComPtr<IDXGIFactory4> factory;
  ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(factory.GetAddressOf())));

  device_ = std::make_unique<dxh::Device>(factory.Get());
  auto* device = device_->Get();

  {

    cmdQueue_ = std::make_unique<dxh::CommandQueue>(device);
    cmdAllocator_ = std::make_unique<dxh::CommandAllocator>(device);
    cmdList_ = std::make_unique<dxh::GraphicsCommandList>(device, cmdAllocator_->Get());
    cmdList_->Close();

    rtvHeap_ = dxh::MakeRTVHeap(device, 10);
    dsvHeap_ = dxh::MakeDSVHeap(device, 10);

    auto swapChainBufferRTVAlloc = [&](int i) { return rtvHeap_->CPUHandle(i); };
    swapChain_ = std::make_unique<dxh::SwapChain>(
      device, factory.Get(), cmdQueue_->Get(), window, 2, static_cast<int>(viewport_.Width),
      static_cast<int>(viewport_.Height), swapChainBufferRTVAlloc
    );

    fence_ = std::make_unique<dxh::Fence>(device);
  }

  // Pipeline
  {

    D3D12_INPUT_ELEMENT_DESC inputElementDesc[] = {
      {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
       0},
      {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
       0},
      {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24,
       D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
    };

    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init(
      0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
    );

    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;
    ThrowIfFailed(D3D12SerializeRootSignature(
      &rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error
    ));
    ThrowIfFailed(device->CreateRootSignature(
      0, signature->GetBufferPointer(), signature->GetBufferSize(),
      IID_PPV_ARGS(rootSignature_.GetAddressOf())
    ));

#if defined(_DEBUG)
    UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
    UINT compileFlags = 0;
#endif

    vs_ = std::make_unique<dxh::VertexShader>(
      L"D:\\Project\\CG\\DirectXHelper\\Test\\shaders.hlsl", "VSMain", compileFlags
    );
    ps_ = std::make_unique<dxh::PixelShader>(
      L"D:\\Project\\CG\\DirectXHelper\\Test\\shaders.hlsl", "PSMain", compileFlags
    );

    D3D12_GRAPHICS_PIPELINE_STATE_DESC desc{};
    desc.InputLayout = {inputElementDesc, _countof(inputElementDesc)};
    desc.pRootSignature = rootSignature_.Get();
    desc.VS = vs_->ByteCode();
    desc.PS = ps_->ByteCode();
    desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    desc.DepthStencilState.DepthEnable = FALSE;
    desc.DepthStencilState.StencilEnable = FALSE;
    desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    desc.NumRenderTargets = 1;
    desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.SampleMask = UINT_MAX;

    DX::ThrowIfFailed(device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(pso_.GetAddressOf()))
    );
  }

  {
    dxh::DefaultTriangle t;
    t.v0 = {XMFLOAT3{0.25f, 0.f, 0.25f}};
    t.v1 = {XMFLOAT3{-0.25f, 0.f, 0.25f}};
    t.v2 = {XMFLOAT3{0.f, 0.25f, 0.25f}};
    t.v0.color = DirectX::XMFLOAT4{1.f, 0.f, 0.f, 1.f};
    t.v1.color = DirectX::XMFLOAT4{1.f, 0.f, 0.f, 1.f};
    t.v2.color = DirectX::XMFLOAT4{0.f, 0.f, 1.f, 1.f};

    vBuffer_ = std::make_unique<dxh::DefaultVertexBuffer>(
      device, *cmdList_, *cmdAllocator_, *cmdQueue_, *fence_, t
    );
    iBuffer_ = std::make_unique<dxh::DefaultIndexBuffer>(
      device, *cmdList_, *cmdAllocator_, *cmdQueue_, *fence_, t
    );
  }

  WaitForGPUCompletion();
}

void D3DDefaultApp::Update()
{
  totalTimeElapsedMs += frameTimeMs;

  // Generate random color
}


void D3DDefaultApp::Render()
{
  DXH_SCOPED_AUTO_TIMER_OUT_RESULT(frameTimeMs, std::chrono::milliseconds);


  dxh::ResetCommandAllocator(*cmdAllocator_);
  dxh::ResetCommandList(*cmdList_, *cmdAllocator_);

  auto* cmdList = cmdList_->Get();


  cmdList->RSSetViewports(1, &viewport_);
  cmdList->RSSetScissorRects(1, &scissorRect_);


  OnRender();

  cmdList->Close();

  auto* cmdQueue = cmdQueue_->Get();
  dxh::ExecuteCommandList(cmdList, cmdQueue);

  swapChain_->Present();
  swapChain_->Swap();

  WaitForGPUCompletion();
}


void D3DDefaultApp::OnRender()
{
  auto* swapChain = swapChain_.get();
  auto backBufferRTV = swapChain->CurrentRTV();
  auto* backBuffer = swapChain->CurrentBuffer();
  auto* cmdList = cmdList_->Get();
  const float clearColor[] = {0.1f, 0.2f, 0.4f, 1.f};

  auto cmds = dxh::MergeCommands(
    dxh::TrackedTransition(backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET),

    dxh::ClearRTV(backBufferRTV, clearColor),

    [&](ID3D12GraphicsCommandList* cmdList) {
      cmdList->SetPipelineState(pso_.Get());
      cmdList->SetGraphicsRootSignature(rootSignature_.Get());
      cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

      auto vbv = vBuffer_->GetVBV();
      cmdList->IASetVertexBuffers(0, 1, &vbv);

      auto ibv = iBuffer_->GetIBV();
      cmdList->IASetIndexBuffer(&ibv);

      cmdList->OMSetRenderTargets(1, &backBufferRTV, false, nullptr);
      cmdList->DrawIndexedInstanced(3, 1, 0, 0, 0);
    },

    dxh::TrackedTransition(backBuffer, D3D12_RESOURCE_STATE_PRESENT)
  );

  dxh::SendToCommandList(cmdList_->Get(), cmds);
}
