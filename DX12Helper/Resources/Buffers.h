#pragma once

#include <functional>

#include "Resources.h"


namespace dxh
{
class GraphicsCommandList;


class Buffer : public TrackedResource
{
public:
  explicit Buffer(
    ID3D12Device* device,
    UINT64 byteSize,
    D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE,
    D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON,
    const CD3DX12_CLEAR_VALUE* clearValue = nullptr,
    D3D12_HEAP_TYPE heapType = D3D12_HEAP_TYPE_DEFAULT,
    D3D12_HEAP_FLAGS heapFlag = D3D12_HEAP_FLAG_NONE,
    UINT64 alignment = 0
  )
      : TrackedResource{device,   CD3DX12_RESOURCE_DESC::Buffer(byteSize, flags, alignment),
                        state,    clearValue,
                        heapType, heapFlag},
        byteSize{byteSize}
  {
  }

  size_t ByteSize() const { return byteSize; }

private:
  size_t byteSize;
};

class UploadHeapBuffer : public Buffer
{
public:
  explicit UploadHeapBuffer(
    ID3D12Device* device,
    size_t byteSize,
    const CD3DX12_CLEAR_VALUE* clearValue = nullptr
  )
      : Buffer{
          device,
          byteSize,
          D3D12_RESOURCE_FLAG_NONE,
          D3D12_RESOURCE_STATE_GENERIC_READ,
          clearValue,
          D3D12_HEAP_TYPE_UPLOAD,
          D3D12_HEAP_FLAG_NONE,
          0
        }
  {
    D3D12_RANGE range = {0, 0};
    Resource()->Map(0, &range, &bufferBegin);
  }
  void Load(size_t dstOffset, const void* srcBegin, size_t srcOffset, size_t byteSize)
  {
    auto* dst = static_cast<UINT8*>(bufferBegin) + dstOffset;
    const auto* src = static_cast<const UINT8*>(srcBegin) + srcOffset;
    memcpy(dst, src, byteSize);
  }

  void Load(const void* srcBegin, size_t byteSize) { Load(0, srcBegin, 0, byteSize); }

  void Clear(int value) const { memset(bufferBegin, value, ByteSize()); }

  ~UploadHeapBuffer()
  {
    D3D12_RANGE range = {0, 0};
    Resource()->Unmap(0, &range);
    bufferBegin = nullptr;
  }

protected:
  void* bufferBegin = nullptr;
};

template<typename ElemType, size_t alignment = sizeof(ElemType)>
class UploadHeapArray : public UploadHeapBuffer
{
public:
  explicit UploadHeapArray(ID3D12Device* device, size_t elemCount);

  size_t ElementPaddedSize() const { return elemPaddedSize; }

  size_t ElementCount() const { return elemCount; }

  void LoadElement(size_t index, const ElemType& elem);

private:
  size_t elemCount = 0;
  int elemPaddedSize = 0;
};

template<typename ElemType, size_t alignment>
UploadHeapArray<ElemType, alignment>::UploadHeapArray(ID3D12Device* device, size_t elemCount)
    : UploadHeapBuffer{device, elemCount * ComputePaddedSize(sizeof(ElemType), alignment)},
      elemCount{elemCount},
      elemPaddedSize{ComputePaddedSize(sizeof(ElemType), alignment)}
{
}

template<typename ElemType, size_t alignment>
void UploadHeapArray<ElemType, alignment>::LoadElement(size_t index, const ElemType& elem)
{
  Load(index * elemPaddedSize, &elem, 0, sizeof(elem));
}

class DefaultHeapBuffer : public Buffer
{
public:
  using CopyCommand = std::function<void(GraphicsCommandList&)>;

  explicit DefaultHeapBuffer(
    ID3D12Device* device,
    size_t byteSize,
    D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON,
    const CD3DX12_CLEAR_VALUE* clearValue = nullptr
  )
      : Buffer{device, byteSize, D3D12_RESOURCE_FLAG_NONE, state, clearValue, D3D12_HEAP_TYPE_DEFAULT, D3D12_HEAP_FLAG_NONE, 0},
        uploader{std::make_unique<UploadHeapBuffer>(device, byteSize, clearValue)}
  {
  }

  void PrepareLoad(size_t dstOffset, const void* srcBegin, size_t srcOffset, size_t byteSize);

  void RecordCopyCommands(GraphicsCommandList& cmdList);

private:
  std::unique_ptr<UploadHeapBuffer> uploader;
  std::vector<CopyCommand> copyCommands;
};


}  // namespace dxh