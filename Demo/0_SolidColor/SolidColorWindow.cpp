#include "CommandAllocator.h"
#include "CommandList.h"
#include "CommandQueue.h"
#include "DescriptorHeap.h"
#include "Device.h"
#include "Fence.h"
#include "PCH.h"
#include "Resources.h"
#include "SwapChain.h"
#include "Timer.h"

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


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

  auto device = std::make_unique<dxh::Device>(factory.Get());
  auto cmdQueue = std::make_unique<dxh::CommandQueue>(device->Get());
  auto swapChain = std::make_unique<dxh::SwapChain<2>>(
    factory.Get(), cmdQueue->Get(), hwnd, 800, 600
  );

  auto rtvHeap = std::make_unique<dxh::DescriptorHeap>(
    device->Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2
  );

  dxh::SwapChainRender<2> swapChainRender(
    device->Get(), *swapChain, {rtvHeap->CPUHandle(0), rtvHeap->CPUHandle(1)}
  );

  auto cmdAlloc = std::make_unique<dxh::CommandAllocator>(device->Get());
  auto cmdList = std::make_unique<dxh::GraphicsCommandList>(device->Get(), cmdAlloc->Get());
  cmdList->Close();
  auto fence = std::make_unique<dxh::Fence>(device->Get());

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

    cmdAlloc->Reset();
    cmdList->Reset(*cmdAlloc);

    auto currentRTV = swapChainRender.CurrentRTV();
    cmdList->Get()->OMSetRenderTargets(1, &currentRTV, FALSE, nullptr);

    ID3D12Resource* currentBuffer = swapChainRender.CurrentBuffer();

    cmdList->Transition(
      currentBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET
    );

    long long t = timer.TimeElapsed("main");
    float r = (sinf(t / 1000.0f) + 1.0f) * 0.5f;
    float g = (cosf(t / 1000.0f) + 1.0f) * 0.5f;
    float b = 0.5f;

    FLOAT clearColor[] = {r, g, b, 1.0f};
    cmdList->Get()->ClearRenderTargetView(currentRTV, clearColor, 0, nullptr);

    cmdList->Transition(
      currentBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT
    );

    cmdList->Close();
    cmdList->Execute(*cmdQueue);

    fence->FlushCommandQueue(cmdQueue->Get());

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