#ifndef DXH_COMMANDLIST_H_
#define DXH_COMMANDLIST_H_

#include <d3d12.h>
#include <wrl/client.h>

#include "DrawCommands.h"

namespace dxh
{

class GraphicsCommandList
{
public:
  explicit GraphicsCommandList(ID3D12Device* device, ID3D12CommandAllocator* alloc);

  ID3D12GraphicsCommandList* Get() const { return cmdList_.Get(); }

  void Close() const { cmdList_->Close(); }

  std::vector<DrawCommands> QueuedDrawCommands() const { return queuedDrawCmds_; }

  void ClearQueuedDrawCommands() { queuedDrawCmds_.clear(); }

  void RecordDrawCommands(const DrawCommands& cmds) { queuedDrawCmds_.push_back(cmds); }

private:
  Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList_;

  std::vector<DrawCommands> queuedDrawCmds_;
};


}  // namespace dxh

#endif  // DXH_COMMANDLIST_H_