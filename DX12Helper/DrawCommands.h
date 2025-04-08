#ifndef DXH_DRAWCOMMANDS_H_
#define DXH_DRAWCOMMANDS_H_

#include <d3d12.h>
#include <functional>
#include <vector>

#include "d3dx12.h"

namespace dxh
{

using DrawCommand = std::function<void(ID3D12GraphicsCommandList* cmdList)>;

class DrawCommands
{
public:
  DrawCommands() = default;
  DrawCommands(DrawCommand cmd) : cmds_{cmd} {}
  DrawCommands(std::vector<DrawCommand> cmds) : cmds_{cmds} {}

  template<typename T>
  DrawCommands(std::initializer_list<T> cmds) : cmds_{cmds}
  {
  }

  auto begin() { return cmds_.begin(); }
  auto end() { return cmds_.end(); }

  auto begin() const { return cmds_.cbegin(); }
  auto end() const { return cmds_.cend(); }

  auto cbegin() const { return cmds_.cbegin(); }
  auto cend() const { return cmds_.cend(); }

  DrawCommands Append(DrawCommand cmd);
  DrawCommands Append(DrawCommands cmds);

  // void operator()(const ID3D12GraphicsCommandList* cmdList) {
    // for (auto& cmd : cmds_) {
      // cmd(cmdList);
    // }
  // }

private:
  std::vector<DrawCommand> cmds_;
};

inline DrawCommands MergeCommands(DrawCommands cmds)
{
  return cmds;
}

inline DrawCommands MergeCommands(DrawCommand cmd)
{
  return cmd;
}

template<typename... Ts>
DrawCommands MergeCommands(DrawCommands cmds, Ts... rest)
{
  return cmds.Append(MergeCommands(rest...));
}

template<typename... Ts>
DrawCommands MergeCommands(DrawCommand cmd, Ts... rest)
{
  return MergeCommands(DrawCommands{cmd}, rest...);
}

DrawCommands MergeCommands(const std::vector<DrawCommands>& drawCmds);

void SendToCommandList(ID3D12GraphicsCommandList* cmdList, const DrawCommands& cmds);

DrawCommands ErrorDrawCommand();

DrawCommands EmptyDrawCommand();

DrawCommands
SetRenderTarget(const D3D12_CPU_DESCRIPTOR_HANDLE& rtv, const D3D12_CPU_DESCRIPTOR_HANDLE& dsv);

DrawCommands ClearDSV(const D3D12_CPU_DESCRIPTOR_HANDLE& dsv, float depth, UINT8 stencil);

DrawCommands ClearRTV(const D3D12_CPU_DESCRIPTOR_HANDLE& rtv, float r, float g, float b, float a);

DrawCommands ClearRTV(const D3D12_CPU_DESCRIPTOR_HANDLE& rtv, const float* rgbaColor);

DrawCommands
SetViewportAndScissorRect(const CD3DX12_VIEWPORT& viewport, const D3D12_RECT& scissorRect);


}  // namespace dxh


#endif  // DXH_DRAWCOMMANDS_H_