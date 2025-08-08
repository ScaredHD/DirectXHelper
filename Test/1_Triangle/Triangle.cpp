#include "CommandAllocator.h"
#include "CommandObjects.h"
#include "CommandQueue.h"
#include "DescriptorHeap.h"
#include "Device.h"
#include "Fence.h"
#include "PCH.h"
#include "Resources.h"
#include "SwapChain.h"
#include "Timer.h"

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

struct Vertex {
  DirectX::XMFLOAT3 position;
  DirectX::XMFLOAT4 color;
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


  dxh::Timer timer;
  timer.Start("main");

  Vertex v0 = {{0.0f, 0.5f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}};
  Vertex v1 = {{-0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}};
  Vertex v2 = {{0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}};
  std::vector<Vertex> vertices = {v0, v1, v2};

  D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
     0},
    {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
     0}
  };

  std::vector<uint16_t> indices = {0, 1, 2};

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