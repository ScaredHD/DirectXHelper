#pragma once

#include "Resources.h"

namespace dxh
{


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

class DefaultHeapBuffer : public Buffer
{
public:
  explicit DefaultHeapBuffer(
    ID3D12Device* device,
    size_t byteSize,
    D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON,
    const CD3DX12_CLEAR_VALUE* clearValue = nullptr
  )
      : Buffer{
          device,
          byteSize,
          D3D12_RESOURCE_FLAG_NONE,
          state,
          clearValue,
          D3D12_HEAP_TYPE_DEFAULT,
          D3D12_HEAP_FLAG_NONE,
          0
        }
  {
  }
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
  }
};

template<typename ElemType, size_t alignment = sizeof(ElemType)>
class UploadHeapArray : public UploadHeapBuffer
{
public:
  explicit UploadHeapArray(ID3D12Device* device, size_t elemCount);
  ~UploadHeapArray();

  size_t ElementPaddedSize() const { return elemPaddedSize_; }

  size_t ElementCount() const { return elemCount_; }

  void Clear(int value) const { memset(bufBegin_, value, BufferByteSize()); }

  void LoadBuffer(size_t byteOffset, const void* dataBegin, size_t byteSize) const;

  void LoadBuffer(const void* dataBegin, size_t byteSize) const
  {
    LoadBuffer(0, dataBegin, byteSize);
  }

  void LoadElement(size_t index, const ElemType& elem);

private:
  void* bufBegin_ = nullptr;
  size_t elemCount_ = 0;
  int elemPaddedSize_ = 0;
};

template<typename ElemType, size_t alignment>
UploadHeapArray<ElemType, alignment>::UploadHeapArray(ID3D12Device* device, size_t elemCount)
    : UploadHeapBuffer{device, elemCount * ComputePaddedSize(sizeof(ElemType), alignment)},
      elemCount_{elemCount},
      elemPaddedSize_{ComputePaddedSize(sizeof(ElemType), alignment)}
{
  D3D12_RANGE range = {0, 0};  // CPU won't read;
  DX::ThrowIfFailed(Resource()->Map(0, &range, &bufBegin_));
}

template<typename ElemType, size_t alignment>
inline UploadHeapArray<ElemType, alignment>::~UploadHeapArray()
{
  D3D12_RANGE range = {0, 0};
  Resource()->Unmap(0, &range);
  bufBegin_ = nullptr;
}

template<typename ElemType, size_t alignment>
void UploadHeapArray<ElemType, alignment>::LoadBuffer(
  size_t byteOffset,
  const void* dataBegin,
  size_t byteSize
) const
{
  auto* dst = static_cast<UINT8*>(bufBegin_) + byteOffset;
  memcpy(dst, dataBegin, byteSize);
}

template<typename ElemType, size_t alignment>
void UploadHeapArray<ElemType, alignment>::LoadElement(size_t index, const ElemType& elem)
{
  auto* p = static_cast<UINT8*>(bufBegin_);
  p += index * elemPaddedSize_;
  memcpy(p, &elem, sizeof(elem));
}


}  // namespace dxh