#include "Buffers.h"

#include "CommandList.h"

namespace dxh
{

void dxh::DefaultHeapBuffer::PrepareLoad(
  size_t dstOffset,
  const void* srcBegin,
  size_t srcOffset,
  size_t byteSize
)
{
  uploader->Load(dstOffset, srcBegin, srcOffset, byteSize);

  auto transition = [this](GraphicsCommandList& cmdList) {
    cmdList.Transition(*this, D3D12_RESOURCE_STATE_COPY_DEST);
  };

  auto copy = [this, dstOffset, srcOffset, byteSize](GraphicsCommandList& cmdList) {
    cmdList.Get()->CopyBufferRegion(
      Resource(), dstOffset, uploader->Resource(), srcOffset, byteSize
    );
  };

  auto transitionBack = [this](GraphicsCommandList& cmdList) {
    cmdList.Transition(*this, PrevState());
  };

  copyCommands.emplace_back(transition);
  copyCommands.emplace_back(copy);
  copyCommands.emplace_back(transitionBack);
}

void dxh::DefaultHeapBuffer::QueueCopyCommands(GraphicsCommandList& cmdList) {
  for (const auto& command : copyCommands) {
    command(cmdList);
  }
}

}  // namespace dxh