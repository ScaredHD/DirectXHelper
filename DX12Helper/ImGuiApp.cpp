#include "ImGuiApp.h"

#include <stdexcept>
#include <string>

#include "D3DApp.h"
#include "imgui.h"
#include "imgui_impl_dx12.h"
#include "imgui_impl_win32.h"

namespace
{


// struct ExampleDescriptorHeapAllocator {
//   ID3D12DescriptorHeap* Heap = nullptr;
//   D3D12_DESCRIPTOR_HEAP_TYPE HeapType = D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES;
//   D3D12_CPU_DESCRIPTOR_HANDLE HeapStartCpu;
//   D3D12_GPU_DESCRIPTOR_HANDLE HeapStartGpu;
//   UINT HeapHandleIncrement;
//   ImVector<int> FreeIndices;

//   void Create(ID3D12Device* device, ID3D12DescriptorHeap* heap)
//   {
//     IM_ASSERT(Heap == nullptr && FreeIndices.empty());
//     Heap = heap;
//     D3D12_DESCRIPTOR_HEAP_DESC desc = heap->GetDesc();
//     HeapType = desc.Type;
//     HeapStartCpu = Heap->GetCPUDescriptorHandleForHeapStart();
//     HeapStartGpu = Heap->GetGPUDescriptorHandleForHeapStart();
//     HeapHandleIncrement = device->GetDescriptorHandleIncrementSize(HeapType);
//     FreeIndices.reserve((int)desc.NumDescriptors);
//     for (int n = desc.NumDescriptors; n > 0; n--)
//       FreeIndices.push_back(n);
//   }
//   void Destroy()
//   {
//     Heap = nullptr;
//     FreeIndices.clear();
//   }
//   void Alloc(D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_desc_handle,
//              D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_desc_handle)
//   {
//     IM_ASSERT(FreeIndices.Size > 0);
//     int idx = FreeIndices.back();
//     FreeIndices.pop_back();
//     out_cpu_desc_handle->ptr = HeapStartCpu.ptr + (idx * HeapHandleIncrement);
//     out_gpu_desc_handle->ptr = HeapStartGpu.ptr + (idx * HeapHandleIncrement);
//   }
//   void Free(D3D12_CPU_DESCRIPTOR_HANDLE out_cpu_desc_handle,
//             D3D12_GPU_DESCRIPTOR_HANDLE out_gpu_desc_handle)
//   {
//     int cpu_idx = (int)((out_cpu_desc_handle.ptr - HeapStartCpu.ptr) / HeapHandleIncrement);
//     int gpu_idx = (int)((out_gpu_desc_handle.ptr - HeapStartGpu.ptr) / HeapHandleIncrement);
//     IM_ASSERT(cpu_idx == gpu_idx);
//     FreeIndices.push_back(cpu_idx);
//   }
// };

// ExampleDescriptorHeapAllocator g_descriptorAlloc;

}  // namespace

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam,
                                                             LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  // if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
  //   return true;

  switch (msg) {
    case WM_SYSCOMMAND:
      if ((wParam & 0xfff0) == SC_KEYMENU)  // Disable ALT application menu
        return 0;
      break;
    case WM_PAINT: {
      PAINTSTRUCT ps;
      HDC hdc = BeginPaint(hWnd, &ps);
      EndPaint(hWnd, &ps);
      break;
    }
    case WM_DESTROY:
      PostQuitMessage(0);
      return 0;
  }
  return DefWindowProcW(hWnd, msg, wParam, lParam);
}

ImGuiApp::ImGuiApp(int width, int height)
{
  WNDCLASSEXW wc = {sizeof(wc),
                    CS_CLASSDC,
                    WndProc,
                    0L,
                    0L,
                    GetModuleHandle(nullptr),
                    nullptr,
                    nullptr,
                    nullptr,
                    nullptr,
                    L"ImGui Example",
                    nullptr};
  ::RegisterClassExW(&wc);
  window = ::CreateWindowW(wc.lpszClassName, L"Dear ImGui DirectX12 Example", WS_OVERLAPPEDWINDOW,
                           100, 100, 1280, 800, nullptr, nullptr, wc.hInstance, nullptr);

  ShowWindow(window, SW_SHOWDEFAULT);
  UpdateWindow(window);

  auto dwStyle = WS_OVERLAPPEDWINDOW;
  RECT rect{0, 0, width, height};
  AdjustWindowRect(&rect, dwStyle, FALSE);

  if (window == NULL) {
    throw std::runtime_error{"Failed to create window"};
  }
}

ImGuiApp::~ImGuiApp()
{
  // ImGui_ImplDX12_Shutdown();
  // ImGui_ImplWin32_Shutdown();
  // ImGui::DestroyContext();
}

void ImGuiApp::Load(D3DDefaultApp* app)
{
  d3dApp = app;
  if (!app) {
    isRunning = false;
    return;
  }

  timer.StartTimer("d3dapp");
}

void ImGuiApp::Run()
{
  MSG msg;
  while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
    if (msg.message == WM_QUIT)
      isRunning = false;
  }

  if (d3dApp && d3dApp->IsRunning()) {
    d3dApp->Update();
    d3dApp->Render();

    UpdateFPSPerSecond();
  }
}

void ImGuiApp::UpdateFPSPerSecond()
{
  if (timer.TimeElapsed("d3dapp") > 1000.f) {
    std::wstring fpsStr{L"fps: "};
    fpsStr += std::to_wstring(1000.f / d3dApp->frameTimeMs);
    SetWindowTextW(window, fpsStr.c_str());
    timer.ResetTimer("d3dapp");
  }
}
