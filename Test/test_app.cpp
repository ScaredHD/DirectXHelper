#include <iostream>
#include <memory>

#include "AutoTimer.h"
#include "D3DApp.h"
#include "ImGuiApp.h"
#include "Timer.h"


int main()
{
  try {
    ImGuiApp guiApp{1280, 720};
    D3DDefaultApp d3dApp(guiApp.Window());
    guiApp.Load(&d3dApp);

    while (guiApp.IsRunning()) {
      guiApp.Run();
    }
  } catch (const std::exception& e) {
    std::cerr << e.what() << '\n';
  }
}