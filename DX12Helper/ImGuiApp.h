#ifndef DXH_IMGUIAPP_H_
#define DXH_IMGUIAPP_H_

class D3DDefaultApp;

#include <Windows.h>

#include "Timer.h"

class ImGuiApp
{
public:
  explicit ImGuiApp(int width, int height);

  ~ImGuiApp();

  void Load(D3DDefaultApp* app);

  void Run();

  HWND Window() const { return window; }

  bool IsRunning() const { return isRunning; }

  void UpdateFPSPerSecond();

private:
  HWND window;

  D3DDefaultApp* d3dApp;

  bool isRunning = true;

  dxh::Timer<std::chrono::milliseconds, std::chrono::high_resolution_clock> timer;
};

inline int ClientWidth(HWND window)
{
  RECT rect;
  GetClientRect(window, &rect);
  return rect.right - rect.left;
}

inline int ClientHeight(HWND window)
{
  RECT rect;
  GetClientRect(window, &rect);
  return rect.bottom - rect.top;
}


#endif  // DXH_IMGUIAPP_H_