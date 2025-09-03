#include "Buffers.h"
#include "CommandAllocator.h"
#include "CommandList.h"
#include "CommandQueue.h"
#include "DescriptorHeap.h"
#include "Device.h"
#include "Fence.h"
#include "PCH.h"
#include "Resources.h"
#include "Shader.h"
#include "SwapChain.h"
#include "Timer.h"


LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

struct VertexF {
  DirectX::XMFLOAT3 position;
  DirectX::XMFLOAT4 color;
};

struct VertexU {
  DirectX::XMFLOAT3 position;
  uint8_t color[4];  // Use uint8_t for color components
};

struct ConstantBuffer {
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

  dxh::Device device{factory.Get()};
  dxh::CommandAllocator cmdAlloc{device.Get()};
  dxh::CommandQueue cmdQueue{device.Get()};
  dxh::SwapChain<2> swapChain{factory.Get(), cmdQueue.Get(), hwnd, 800, 600};

  dxh::DescriptorHeap rtvHeap{device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2};
  dxh::SwapChainRender<2> swapChainRender{
    device.Get(), swapChain, {rtvHeap.CPUHandle(0), rtvHeap.CPUHandle(1)}
  };

  dxh::Fence fence{device.Get()};
  dxh::GraphicsCommandList cmdList{device.Get(), cmdAlloc.Get()};


#define USE_UNORM

#if defined(USE_UNORM)
  using Vertex = VertexU;
  VertexU v0u = {{0.0f, 0.5f, 0.0f}, {255, 0, 0, 255}};
  VertexU v1u = {{-0.5f, -0.5f, 0.0f}, {0, 255, 0, 255}};
  VertexU v2u = {{0.5f, -0.5f, 0.0f}, {0, 0, 255, 255}};
  std::vector<Vertex> vertices = {v0u, v1u, v2u};
#else
  using Vertex = VertexF;
  VertexF v0 = {{0.0f, 0.5f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}};
  VertexF v1 = {{-0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}};
  VertexF v2 = {{0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}};
  std::vector<VertexF> vertices = {v0, v1, v2};
#endif

  size_t vertexBufferSize = vertices.size() * sizeof(Vertex);
  dxh::DefaultHeapBuffer vertexBuffer{device.Get(), vertexBufferSize};
  vertexBuffer.PrepareLoad(0, vertices.data(), 0, vertexBufferSize);
  vertexBuffer.QueueCopyCommands(cmdList);

  std::vector<uint16_t> indices = {0, 1, 2};

  size_t indexBufferSize = indices.size() * sizeof(uint16_t);
  dxh::DefaultHeapBuffer indexBuffer{device.Get(), indexBufferSize};
  indexBuffer.PrepareLoad(0, indices.data(), 0, indexBufferSize);
  indexBuffer.QueueCopyCommands(cmdList);

  cmdList.Close();
  cmdList.Execute(cmdQueue.Get());
  fence.FlushCommandQueue(cmdQueue.Get());


  dxh::UploadHeapArray<ConstantBuffer> constantBuffer{device.Get(), 1};

  D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
     0},
#if defined(USE_UNORM)
    {"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
#else
    {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
     0}
#endif
  };



  Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
  {
    CD3DX12_ROOT_PARAMETER rootParameters[1];
    rootParameters[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL);

    CD3DX12_ROOT_SIGNATURE_DESC desc{};
    desc.Init(
      1, rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
    );

    Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
    DX::ThrowIfFailed(D3D12SerializeRootSignature(
      &desc, D3D_ROOT_SIGNATURE_VERSION_1_0, signatureBlob.GetAddressOf(), errorBlob.GetAddressOf()
    ));

    device.Get()->CreateRootSignature(
      0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(),
      IID_PPV_ARGS(rootSignature.ReleaseAndGetAddressOf())
    );
  }

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

    desc.InputLayout = {inputLayout, _countof(inputLayout)};
    desc.pRootSignature = rootSignature.Get();
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
    ConstantBuffer cb;
    cb.time = static_cast<float>(time) / 1000.0f;  // Convert to seconds
    constantBuffer.LoadElement(0, cb);

    cmdAlloc.Reset();
    cmdList.Reset(cmdAlloc.Get());

    cmdList.Transition(
      swapChainRender.CurrentBuffer(), D3D12_RESOURCE_STATE_PRESENT,
      D3D12_RESOURCE_STATE_RENDER_TARGET
    );

    cmdList.Get()->SetGraphicsRootSignature(rootSignature.Get());
    cmdList.Get()->SetPipelineState(pso.Get());

    cmdList.Get()->SetGraphicsRootConstantBufferView(0, constantBuffer.GPUVirtualAddress());

    cmdList.Get()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    auto vbv = D3D12_VERTEX_BUFFER_VIEW{
      vertexBuffer.GPUVirtualAddress(), static_cast<UINT>(vertexBuffer.ByteSize()), sizeof(Vertex)
    };
    cmdList.Get()->IASetVertexBuffers(0, 1, &vbv);

    auto ibv = D3D12_INDEX_BUFFER_VIEW{
      indexBuffer.GPUVirtualAddress(), static_cast<UINT>(indexBuffer.ByteSize()),
      DXGI_FORMAT_R16_UINT
    };
    cmdList.Get()->IASetIndexBuffer(&ibv);

    auto viewport = dxh::MakeViewport(swapChain);
    cmdList.Get()->RSSetViewports(1, &viewport);
    auto scissorRect = dxh::MakeScissorRect(swapChain);
    cmdList.Get()->RSSetScissorRects(1, &scissorRect);

    auto rtv = swapChainRender.CurrentRTV();
    cmdList.Get()->OMSetRenderTargets(1, &rtv, FALSE, nullptr);

    FLOAT bgColor[] = {0.1f, 0.2f, 0.4f, 1.f};
    cmdList.Get()->ClearRenderTargetView(rtv, bgColor, 0, nullptr);

    cmdList.Get()->DrawIndexedInstanced(3, 1, 0, 0, 0);

    cmdList.Transition(
      swapChainRender.CurrentBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET,
      D3D12_RESOURCE_STATE_PRESENT
    );

    cmdList.Close();
    cmdList.Execute(cmdQueue.Get());

    fence.FlushCommandQueue(cmdQueue.Get());
    swapChainRender.Present();
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
