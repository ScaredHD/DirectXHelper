#include "DrawCommands.h"

#include <algorithm>
#include <iterator>
#include <stdexcept>

#include "D3DUtils.h"
#include "Resources.h"

namespace dxh
{
DrawCommands DrawCommands::Append(DrawCommand cmd)
{
  cmds_.push_back(cmd);
  return *this;
}

DrawCommands DrawCommands::Append(DrawCommands cmds)
{
  std::copy(std::begin(cmds), std::end(cmds), std::back_inserter(cmds_));
  return *this;
}

DrawCommands MergeCommands(const std::vector<DrawCommands>& drawCmds)
{
  DrawCommands res;
  for (const auto& cmds : drawCmds) {
    res.Append(cmds);
  }
  return res;
}

void SendToCommandList(ID3D12GraphicsCommandList* cmdList, const DrawCommands& cmds)
{
  for (const auto& cmd : cmds) {
    cmd(cmdList);
  }
}

DrawCommands ErrorDrawCommand()
{
  return [](ID3D12GraphicsCommandList* cmdList) { throw std::runtime_error{"error draw command"}; };
}

DrawCommands EmptyDrawCommand()
{
  return [](ID3D12GraphicsCommandList* cmdList) {};
}

DrawCommands
SetRenderTarget(const D3D12_CPU_DESCRIPTOR_HANDLE& rtv, const D3D12_CPU_DESCRIPTOR_HANDLE& dsv)
{
  return
    [=](ID3D12GraphicsCommandList* cmdList) { cmdList->OMSetRenderTargets(1, &rtv, true, &dsv); };
}

DrawCommands CloseCommandList()
{
  return [](ID3D12GraphicsCommandList* cmdList) { DX::ThrowIfFailed(cmdList->Close()); };
}

DrawCommands ClearDSV(const D3D12_CPU_DESCRIPTOR_HANDLE& dsv, float depth, UINT8 stencil)
{
  return [=](ID3D12GraphicsCommandList* cmdList) {
    cmdList->ClearDepthStencilView(
      dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, depth, stencil, 0, nullptr
    );
  };
}

DrawCommands ClearRTV(const D3D12_CPU_DESCRIPTOR_HANDLE& rtv, float r, float g, float b, float a)
{
  return [=](ID3D12GraphicsCommandList* cmdList) {
    FLOAT color[] = {r, g, b, a};
    cmdList->ClearRenderTargetView(rtv, color, 0, nullptr);
  };
}

DrawCommands ClearRTV(const D3D12_CPU_DESCRIPTOR_HANDLE& rtv, const float* rgbaColor)
{
  return [=](ID3D12GraphicsCommandList* cmdList) {
    cmdList->ClearRenderTargetView(rtv, rgbaColor, 0, nullptr);
  };
}

DrawCommands
SetViewportAndScissorRect(const CD3DX12_VIEWPORT& viewport, const D3D12_RECT& scissorRect)
{
  return [&](ID3D12GraphicsCommandList* cmdList) {
    cmdList->RSSetViewports(1, &viewport);
    cmdList->RSSetScissorRects(1, &scissorRect);
  };
}


}  // namespace dxh
