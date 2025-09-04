#include "Camera.h"
#include "CommandAllocator.h"
#include "CommandList.h"
#include "CommandQueue.h"
#include "DescriptorHeap.h"
#include "Device.h"
#include "Fence.h"
#include "GeometryRender.h"
#include "MeshFactory.h"
#include "PCH.h"
#include "RenderContext.h"
#include "Resources.h"
#include "Shader.h"
#include "SimpleVertex.h"
#include "SwapChain.h"
#include "Textures.h"
#include "Timer.h"


LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

struct ConstantBuffer {
  DirectX::XMFLOAT4X4 view;
  DirectX::XMFLOAT4X4 projection;
  float time;
};

using Vertex = dxh::SimpleVertex;

DirectX::XMFLOAT3 CameraPosition(float time)
{
  float radius = 5.0f;
  float x = radius * cosf(time);
  float z = radius * sinf(time);
  return {x, 2.0f, z};
}


int WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
  // Create Win32 application window
  WNDCLASSEX wc = {
    sizeof(WNDCLASSEX),
    CS_CLASSDC,
    WindowProc,
    0L,
    0L,
    GetModuleHandle(nullptr),
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    "DX12Demo",
    nullptr
  };
  RegisterClassEx(&wc);
  HWND hwnd = CreateWindow(
    wc.lpszClassName, "DX12 Demo", WS_OVERLAPPEDWINDOW, 100, 100, 800, 600, nullptr, nullptr,
    wc.hInstance, nullptr
  );
  ShowWindow(hwnd, nCmdShow);

  // Initialize Direct3D 12
  Microsoft::WRL::ComPtr<IDXGIFactory4> factory;
  HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(factory.ReleaseAndGetAddressOf()));
  if (FAILED(hr)) {
    MessageBox(nullptr, "Failed to create DXGI factory", "Error", MB_OK | MB_ICONERROR);
    return -1;
  }

#if defined(_DEBUG)
  Microsoft::WRL::ComPtr<ID3D12Debug1> debugController;
  if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(debugController.ReleaseAndGetAddressOf())))) {
    debugController->EnableDebugLayer();
    debugController->SetEnableGPUBasedValidation(true);
  }
#endif

  dxh::RenderContext rc{factory.Get(), hwnd, 800, 600};

  dxh::TriangleMeshData<Vertex, uint16_t> boxMeshData = dxh::CreateUnitBox<Vertex, uint16_t>();
  dxh::TriangleMeshRenderResource<Vertex, uint16_t> boxMesh{rc.device->Get(), &boxMeshData};

  dxh::CommandAllocator cmdAlloc{rc.device->Get()};
  dxh::GraphicsCommandList cmdList{rc.device->Get(), cmdAlloc.Get()};

  boxMesh.QueueUploadMeshData(cmdList);
  rc.CloseAndExecute(cmdList);
  rc.FlushCommandQueue();

  dxh::UploadHeapArray<ConstantBuffer> constantBuffer{rc.device->Get(), 1};
  auto cbv = rc.cbvSrvUavPool.Allocate();
  rc.device->CreateCBV(constantBuffer, cbv);

  CD3DX12_DESCRIPTOR_RANGE ranges[1];
  ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0);

  CD3DX12_ROOT_PARAMETER rootParams[1];
  rootParams[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_ALL);

  dxh::RootSignature rs{rc.device->Get(), 1, rootParams};

  dxh::DynamicDescriptorHeap dynamicDescriptorHeap{
    rc.device->Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
  };

  dynamicDescriptorHeap.ParseRootSignature(rs);
  dynamicDescriptorHeap.SetDescriptors(0, 0, 1, &cbv);

  dxh::VertexShader vs{L"shaders.hlsl", "MainVS", 0};
  dxh::PixelShader ps{L"shaders.hlsl", "MainPS", 0};

  D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
  psoDesc.InputLayout = {Vertex::inputLayout, Vertex::inputLayoutCount};
  psoDesc.pRootSignature = rs.GetRootSignature();
  psoDesc.VS = D3D12_SHADER_BYTECODE(vs.ByteCode());
  psoDesc.PS = D3D12_SHADER_BYTECODE(ps.ByteCode());
  psoDesc.RTVFormats[0] = rc.swapChain->Format();
  psoDesc.NumRenderTargets = 1;
  psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
  psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
  psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
  psoDesc.SampleDesc = {1, 0};
  psoDesc.SampleMask = UINT_MAX;
  psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
  psoDesc.DSVFormat = rc.swapChainManager->DepthBufferFormat();


  Microsoft::WRL::ComPtr<ID3D12PipelineState> pso;
  rc.device->Get()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(pso.GetAddressOf()));

  dxh::PerspectiveCamera cam;

  dxh::Timer timer;
  timer.Start("main");

  bool isRunning = true;
  while (isRunning) {
    MSG msg;
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
      if (msg.message == WM_QUIT) {
        isRunning = false;
        continue;
      }
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }

    long long time = timer.TimeElapsed("main");

    cam.position = CameraPosition(static_cast<float>(time) / 1000.0f);
    cam.lookAt = {0.0f, 0.0f, 0.0f};
    cam.up = {0.0f, 1.0f, 0.0f};

    ConstantBuffer cb;
    cb.view = cam.ViewMatrix();
    cb.projection = cam.ProjectionMatrix();
    cb.time = static_cast<float>(time) / 1000.0f;
    constantBuffer.LoadElement(0, cb);

    cmdAlloc.Reset();
    cmdList.Reset(cmdAlloc);

    cmdList.SetRootSignature(rs);
    cmdList.SetPipelineState(pso.Get());

    dynamicDescriptorHeap.BindModifiedDescriptors(rc.device->Get(), cmdList.Get());

    cmdList.SetViewport(*rc.swapChain);
    cmdList.SetScissorRect(*rc.swapChain);

    auto dsv = rc.swapChainManager->CurrentDSV();
    rc.PrepareSwapChainForRender(cmdList, dsv);

    rc.ClearBackBuffer(cmdList, {0.2f, 0.3f, 0.3f, 1.0f});

    cmdList.SetTriangleMeshToDraw(boxMesh);
    cmdList.DrawTriangleMeshResource(boxMesh);

    rc.PrepareSwapChainForPresent(cmdList);
    rc.CloseAndExecute(cmdList);
    rc.FlushCommandQueue();
    rc.Present();
  }
}


LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg) {
    case WM_DESTROY:
      PostQuitMessage(0);
      return 0;
  }
  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
