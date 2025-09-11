#include <iomanip>
#include <sstream>

#include "AutoTimer.h"
#include "Camera.h"
#include "CommandAllocator.h"
#include "CommandList.h"
#include "CommandQueue.h"
#include "DescriptorHeap.h"
#include "Device.h"
#include "Fence.h"
#include "GeometryRender.h"
#include "Instances.h"
#include "MeshFactory.h"
#include "PCH.h"
#include "RenderContext.h"
#include "Resources.h"
#include "Shader.h"
#include "SimpleVertex.h"
#include "SwapChain.h"
#include "Textures.h"
#include "Timer.h"


using namespace DirectX;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

struct ConstantBufferData {
  XMFLOAT4X4 view;
  XMFLOAT4X4 projection;
  XMFLOAT3 lightColor;
  float time;
  XMFLOAT3 lightDir;
  float _padding1;  // HLSL is 16-byte aligned, so add explicit padding
  XMFLOAT3 ambient;
};

using Vertex = dxh::SimpleVertex;

int g_screenWidth = 800;
int g_screenHeight = 600;

bool g_enableFrustumCulling = true;
bool g_enableOctreeCulling = true;
bool g_tickInstances = true;

std::string GetModeString()
{
  std::string res = g_enableFrustumCulling ? "Cull" : "NoCull";
  res += ", ";
  if (g_enableOctreeCulling) {
    if (g_tickInstances) {
      res += "DynamicOctree";
    } else {
      res += "StaticOctree";
    }
  } else {
    res += "NoAcc";
  }
  return res;
}

XMFLOAT3
LightColor(float intensity = 1.0f, const XMFLOAT3& baseColor = {1.0f, 1.0f, 1.0f})
{
  float r = baseColor.x * intensity;
  float g = baseColor.y * intensity;
  float b = baseColor.z * intensity;
  return {r, g, b};
}

XMFLOAT3 g_ambientColor = {0.2f, 0.2f, 0.2f};

XMFLOAT3 LightDirection(float time, float speed = 0.5f)
{
  float x = std::cosf(time * speed);
  float z = std::sinf(time * speed);
  return {x, 1.0f, z};
}

XMFLOAT3 CameraPosition(float time, float speed = 0.5f)
{
  float radius = 40.0f;
  float height = 10.0f;
  float x = radius * cosf(time * speed);
  float z = radius * sinf(time * speed);
  return {x, height, z};
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
    wc.lpszClassName, "DX12 Demo", WS_OVERLAPPEDWINDOW, 100, 100, g_screenWidth, g_screenHeight,
    nullptr, nullptr, wc.hInstance, nullptr
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

  dxh::RenderContext rc{factory.Get(), hwnd, g_screenWidth, g_screenHeight};

  dxh::TriangleMeshData<Vertex, uint16_t> boxMeshData =
    dxh::CreateUnitBoxWithNormal<Vertex, uint16_t>();
  dxh::TriangleMeshRenderResource<Vertex, uint16_t> boxMesh{rc.device->Get(), &boxMeshData};

  dxh::CommandAllocator cmdAlloc{rc.device->Get()};
  dxh::GraphicsCommandList cmdList{rc.device->Get(), cmdAlloc.Get()};

  boxMesh.QueueUploadMeshData(cmdList);
  rc.CloseAndExecute(cmdList);
  rc.FlushCommandQueue();

  dxh::ConstantBuffer<ConstantBufferData> constantBuffer{rc.device->Get(), 1};
  auto cbv = rc.cbvSrvUavPool.Allocate();
  rc.device->CreateCBV(constantBuffer, cbv);

  dxh::UploadHeapArray<InstanceData> instanceBuffer{rc.device->Get(), g_instanceCount};
  auto instanceSRV = rc.cbvSrvUavPool.Allocate();
  rc.device->CreateSRV(instanceBuffer, instanceSRV);

  CD3DX12_DESCRIPTOR_RANGE ranges[2];
  ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0);
  ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);

  CD3DX12_ROOT_PARAMETER rootParams[1];
  rootParams[0].InitAsDescriptorTable(2, ranges, D3D12_SHADER_VISIBILITY_ALL);

  dxh::RootSignature rs{rc.device->Get(), 1, rootParams};

  dxh::DynamicDescriptorHeap dynamicDescriptorHeap{
    rc.device->Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
  };

  dynamicDescriptorHeap.ParseRootSignature(rs);
  dynamicDescriptorHeap.SetDescriptors(0, 0, 1, &cbv);
  dynamicDescriptorHeap.SetDescriptors(0, 1, 1, &instanceSRV);

  dxh::VertexShader vs{L"shaders.hlsl", "MainVS", D3DCOMPILE_DEBUG};
  dxh::PixelShader ps{L"shaders.hlsl", "MainPS", D3DCOMPILE_DEBUG};

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
  cam.aspectRatio = rc.swapChain->Width() / static_cast<float>(rc.swapChain->Height());

  dxh::Timer timer;
  timer.Start("main");

  dxh::Timer<dxh::Microseconds> frameTimer;
  frameTimer.Start("frame");

  long long lastTimeStamp = 0;
  float statPeriodMs = 500.f;  // stat update period in ms
  size_t accumulatedFrames = 0;

  bool isRunning = true;
  while (isRunning) {
    float frameStart = frameTimer.TimeElapsed("frame");
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
    float timeSec = static_cast<float>(time) / 1000.0f;
    ++accumulatedFrames;

    cam.position = CameraPosition(timeSec, 0.1f);
    cam.lookAt = {0.0f, 0.0f, 0.0f};
    cam.up = {0.0f, 1.0f, 0.0f};

    ConstantBufferData cb;
    cb.view = cam.ViewMatrix();
    cb.projection = cam.ProjectionMatrix();
    cb.time = timeSec;
    cb.lightColor = LightColor(1.0f, {1.0f, 1.0f, 1.0f});
    cb.lightDir = LightDirection(timeSec, 2.f);
    cb.ambient = g_ambientColor;
    constantBuffer.LoadElement(0, cb);

    if (g_tickInstances) {
      timer.Start("instances");
      long long instanceTime = timer.TimeElapsed("instances");
      float instanceTimeSec = static_cast<float>(instanceTime) / 1000.0f;
      UpdateInstances(instanceTimeSec);
    } else {
      timer.Pause("instances");
    }

    long long cullTime = 0;
    if (g_enableFrustumCulling) {
      DXH_SCOPED_AUTO_TIMER_OUT_RESULT(cullTime, dxh::Microseconds)

      FrustumCullingSpace space = FrustumCullingSpace::World;

      CullingAcceleration acc;
      if (g_enableOctreeCulling) {
        acc = g_tickInstances ? CullingAcceleration::DynamicOctree
                              : CullingAcceleration::StaticOctree;
      } else {
        acc = CullingAcceleration::None;
      }

      CullInstances(cam, space, acc);
    }

    size_t instanceDrawCount = g_enableFrustumCulling ? g_culledInstanceIndices.size()
                                                      : g_instanceCount;

    for (size_t i = 0; i < instanceDrawCount; ++i) {
      instanceBuffer.LoadElement(
        i, g_instanceBuffer[g_enableFrustumCulling ? g_culledInstanceIndices[i] : i]
      );
    }

    auto elapsedSinceLastStampMs = static_cast<float>(time - lastTimeStamp);
    float elapsedSinceLastStampSec = elapsedSinceLastStampMs / 1000.0f;
    if (elapsedSinceLastStampMs >= statPeriodMs) {
      // update fps every second
      float fps = static_cast<float>(accumulatedFrames) / elapsedSinceLastStampSec;
      std::ostringstream oss;
      oss << "DX12 Instancing Demo";
      oss << " | ";
      oss << "FPS: " << std::fixed << std::setprecision(2) << fps;
      oss << " | ";
      oss << "Inst. drawed: " << instanceDrawCount << "/" << g_instanceCount;
      oss << " | ";
      oss << "Culltime: " << std::setprecision(3) << static_cast<float>(cullTime) / 1000.f << " ms";
      oss << " | Mode: [" << GetModeString() << "]";

      SetWindowText(hwnd, oss.str().c_str());
      accumulatedFrames = 0;
      lastTimeStamp = time;
    }


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

    cmdList.Get()->DrawIndexedInstanced(boxMeshData.IndexCount(), instanceDrawCount, 0, 0, 0);

    rc.PrepareSwapChainForPresent(cmdList);
    rc.CloseAndExecute(cmdList);
    rc.FlushCommandQueue();
    rc.Present();

    float frameEnd = frameTimer.TimeElapsed("frame");
    frameTimer.Reset("frame");
    auto logger = spdlog::get("instance_logger");
    logger->info("Frame time: {} ms", (frameEnd - frameStart) / 1000.f);
  }
}


LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg) {
    case WM_KEYDOWN:
      if (wParam == 'C') {
        g_enableFrustumCulling = !g_enableFrustumCulling;
      }
      if (wParam == 'O') {
        g_enableOctreeCulling = !g_enableOctreeCulling;
        if (g_enableOctreeCulling) {
          g_octreeBuilt = false;
        }
      }
      if (wParam == 'M') {
        g_tickInstances = !g_tickInstances;
      }
      return 0;
    case WM_DESTROY:
      PostQuitMessage(0);
      return 0;
  }
  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
