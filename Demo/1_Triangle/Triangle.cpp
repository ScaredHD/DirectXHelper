#include "Buffers.h"
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
#include "ResourceManager.h"
#include "Resources.h"
#include "RootSignature.h"
#include "Shader.h"
#include "SwapChain.h"
#include "Timer.h"
#include "SimpleVertex.h"



LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

using Vertex = dxh::SimpleVertex;

struct ConstantBufferData {
  float time;
};


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
  auto& device = *rc.device;
  auto& swapChain = *rc.swapChain;

  dxh::CommandAllocator cmdAlloc{device.Get()};
  dxh::GraphicsCommandList cmdList{device.Get(), cmdAlloc.Get()};

  Vertex v0 = {{0.0f, 0.5f, 0.0f}, {255, 0, 0, 255}};
  Vertex v1 = {{-0.5f, -0.5f, 0.0f}, {0, 255, 0, 255}};
  Vertex v2 = {{0.5f, -0.5f, 0.0f}, {0, 0, 255, 255}};
  dxh::TriangleMeshData<Vertex, uint16_t> triangleMeshData = dxh::CreateTriangle<Vertex, uint16_t>(
    v0, v1, v2
  );

  dxh::TriangleMeshRenderResource<Vertex, uint16_t> triangleMeshResource{
    device.Get(), &triangleMeshData
  };

  triangleMeshResource.QueueUploadMeshData(cmdList);

  rc.CloseAndExecute(cmdList);
  rc.FlushCommandQueue();

  CD3DX12_ROOT_PARAMETER rootParameters[2];
  rootParameters[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL);

  D3D12_DESCRIPTOR_RANGE rg{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1, 0, 0};
  rootParameters[1].InitAsDescriptorTable(1, &rg, D3D12_SHADER_VISIBILITY_ALL);

  dxh::DescriptorPool<D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE>
    cbvPool{device.Get()};
  auto cbv = cbvPool.Allocate();

  dxh::ConstantBuffer<ConstantBufferData> constantBuffer{device.Get(), 1};
  device.CreateCBV(constantBuffer, cbv);

  dxh::RootSignature rs{device.Get(), 2, rootParameters};

  dxh::DynamicDescriptorHeap dynamicHeap{device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV};

  dynamicHeap.ParseRootSignature(rs);
  dynamicHeap.SetDescriptors(1, 0, 1, &cbv);

  dxh::VertexShader vertexShader{L"shader.hlsl", "MainVS", 0};
  dxh::PixelShader pixelShader{L"shader.hlsl", "MainPS", 0};

  Microsoft::WRL::ComPtr<ID3D12PipelineState> pso;
  {
    D3D12_GRAPHICS_PIPELINE_STATE_DESC desc{};
    desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    desc.SampleMask = UINT_MAX;

    // Set up rasterizer state with no culling
    desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    desc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

    // Disable depth testing since we don't have a depth buffer
    desc.DepthStencilState.DepthEnable = FALSE;
    desc.DepthStencilState.StencilEnable = FALSE;

    desc.InputLayout = {Vertex::inputLayout, Vertex::inputLayoutCount};
    desc.pRootSignature = rs.GetRootSignature();
    desc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.ByteCode());
    desc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.ByteCode());
    desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    desc.NumRenderTargets = 1;
    desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;

    DX::ThrowIfFailed(
      device.Get()->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(pso.ReleaseAndGetAddressOf()))
    );
  }

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

    // Render
    long long time = timer.TimeElapsed("main");
    ConstantBufferData cb;
    cb.time = static_cast<float>(time) / 1000.0f;  // Convert to seconds
    constantBuffer.LoadElement(0, cb);

    cmdAlloc.Reset();
    cmdList.Reset(cmdAlloc.Get());

    rc.PrepareSwapChainForRender(cmdList);

    cmdList.SetRootSignature(rs);
    cmdList.SetPipelineState(pso.Get());

    cmdList.SetRootCBV(0, constantBuffer.Resource());

    dynamicHeap.BindModifiedDescriptors(device.Get(), cmdList.Get());

    rc.ClearBackBuffer(cmdList, {0.1f, 0.2f, 0.4f, 1.f});

    cmdList.SetTriangleMeshToDraw(triangleMeshResource);

    cmdList.SetViewport(swapChain);
    cmdList.SetScissorRect(swapChain);

    cmdList.Get()->DrawIndexedInstanced(3, 1, 0, 0, 0);

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
