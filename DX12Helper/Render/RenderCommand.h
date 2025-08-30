#pragma once

#include <functional>
namespace dxh
{


class RenderCommand
{
public:
  RenderCommand(std::function<void(ID3D12GraphicsCommandList*)> cmd) : cmd(std::move(cmd)) {}

  void Execute(ID3D12GraphicsCommandList* cmdList) const { cmd(cmdList); }

private:
  std::function<void(ID3D12GraphicsCommandList*)> cmd;
};

class RenderCommandPack
{
public:
  template<typename T>
  void Append(T&& cmd)
  {
    cmds.emplace_back(std::forward<T>(cmd));
  }


  void Execute(ID3D12GraphicsCommandList* cmdList)
  {
    for (const auto& cmd : cmds) {
      cmd.Execute(cmdList);
    }
  }

private:
  std::vector<RenderCommand> cmds;
};

}  // namespace dxh