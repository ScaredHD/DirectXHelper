
#pragma once

#include "PCH.h"

#include "Views.h"


namespace dxh
{
CD3DX12_CLEAR_VALUE ZeroClearValue(DXGI_FORMAT format);

CD3DX12_CLEAR_VALUE BlackClearValue(DXGI_FORMAT format);

CD3DX12_CLEAR_VALUE RGBAClearValue(DXGI_FORMAT format, float r, float g, float b, float a);

CD3DX12_CLEAR_VALUE DefaultDepthStencilClearValue(DXGI_FORMAT format);

int ComputePaddedSize(int size, int alignment);


class RawResource
{
public:
  RawResource() = default;
  RawResource(
    ID3D12Device* device,
    const D3D12_RESOURCE_DESC& desc,
    D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON,
    const D3D12_CLEAR_VALUE* clearValue = nullptr,
    D3D12_HEAP_TYPE heapType = D3D12_HEAP_TYPE_DEFAULT,
    D3D12_HEAP_FLAGS heapFlag = D3D12_HEAP_FLAG_NONE
  );

  ID3D12Resource* Resource() const { return resource_.Get(); }
  D3D12_GPU_VIRTUAL_ADDRESS GPUVirtualAddress() const { return resource_->GetGPUVirtualAddress(); }

  D3D12_RESOURCE_STATES CurrentState() const { return state_; }
  D3D12_RESOURCE_STATES PreviousState() const { return prevState_; }

  void MakeTransition(D3D12_RESOURCE_STATES newState)
  {
    prevState_ = state_;
    state_ = newState;
  }

  ID3D12Resource** GetAddressOf() { return resource_.GetAddressOf(); }

  std::string Name() const { return name_ + "(id: " + std::to_string(id_) + ")"; }
  void SetName(std::string name) { name_ = std::move(name); }

  bool ShouldDebug() const { return debugEnabled_; }
  void SetDebug(bool shouldDebug) { debugEnabled_ = shouldDebug; }

private:
  Microsoft::WRL::ComPtr<ID3D12Resource> resource_;
  D3D12_RESOURCE_STATES state_ = D3D12_RESOURCE_STATE_COMMON;
  D3D12_RESOURCE_STATES prevState_ = D3D12_RESOURCE_STATE_COMMON;

  std::string name_ = "unnamed RawResource";
  int id_ = 0;
  bool debugEnabled_ = false;
};


class BufferResource : public RawResource
{
public:
  BufferResource(
    ID3D12Device* device,
    UINT64 byteSize,
    D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE,
    D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON,
    const CD3DX12_CLEAR_VALUE* clearValue = nullptr,
    D3D12_HEAP_TYPE heapType = D3D12_HEAP_TYPE_DEFAULT,
    D3D12_HEAP_FLAGS heapFlag = D3D12_HEAP_FLAG_NONE,
    UINT64 alignment = 0
  )
      : RawResource{device,   CD3DX12_RESOURCE_DESC::Buffer(byteSize, flags, alignment),
                    state,    clearValue,
                    heapType, heapFlag},
        byteSize_{byteSize}
  {
  }

  size_t BufferByteSize() const { return byteSize_; }

private:
  size_t byteSize_;
};

class DefaultHeapBuffer : public BufferResource
{
public:
  explicit DefaultHeapBuffer(
    ID3D12Device* device,
    size_t byteSize,
    D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON,
    const CD3DX12_CLEAR_VALUE* clearValue = nullptr
  )
      : BufferResource{
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


class UploadHeapBuffer : public BufferResource
{
public:
  explicit UploadHeapBuffer(
    ID3D12Device* device,
    size_t byteSize,
    const CD3DX12_CLEAR_VALUE* clearValue = nullptr
  )
      : BufferResource{
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

template<typename ElemType, int alignment>
ConstantBufferView CreateCBVOnElement(
  ID3D12Device* device,
  const UploadHeapArray<ElemType, alignment>& uploadBuffer,
  size_t elementIdx,
  D3D12_CPU_DESCRIPTOR_HANDLE outCPUHandle
)
{
  auto elementByteSize = uploadBuffer.ElementPaddedSize();
  auto bufferLocation = uploadBuffer.ElementGpuVirtualAddress(elementIdx);
  return {device, bufferLocation, elementByteSize, outCPUHandle};
}


class Texture2D : public RawResource
{
public:
  explicit Texture2D(
    ID3D12Device* device,
    D3D12_RESOURCE_STATES state,
    const D3D12_CLEAR_VALUE* clearValue,
    DXGI_FORMAT format,
    UINT64 width,
    UINT height,
    UINT16 arraySize = 1,
    UINT16 mipLevels = 0,
    UINT sampleCount = 1,
    UINT sampleQuality = 0,
    D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE,
    D3D12_TEXTURE_LAYOUT layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
    UINT64 alignment = 0
  )
      : RawResource(
          device,
          CD3DX12_RESOURCE_DESC::Tex2D(
            format,
            width,
            height,
            arraySize,
            mipLevels,
            sampleCount,
            sampleQuality,
            flags,
            layout,
            alignment
          ),
          state,
          clearValue,
          D3D12_HEAP_TYPE_DEFAULT,
          D3D12_HEAP_FLAG_NONE
        ),
        format_{format},
        width_{width},
        height_{height}
  {
  }

  UINT64 Width() const { return width_; }

  UINT64 Height() const { return height_; }

  DXGI_FORMAT Format() const { return format_; }

private:
  UINT64 width_;
  UINT64 height_;
  DXGI_FORMAT format_;
};

std::unique_ptr<Texture2D>
CreateRenderTargetTexture2D(ID3D12Device* device, DXGI_FORMAT format, UINT64 width, UINT height);

std::unique_ptr<Texture2D>
CreateDepthStencilTexture2D(ID3D12Device* device, DXGI_FORMAT format, UINT64 width, UINT height);

struct DepthStencilBuffer {
  explicit DepthStencilBuffer(
    std::unique_ptr<Texture2D> texture,
    std::unique_ptr<DepthStencilView> dsv
  )
      : texture{std::move(texture)},
        dsv{std::move(dsv)}
  {
  }
  std::unique_ptr<Texture2D> texture;
  std::unique_ptr<DepthStencilView> dsv;
};

std::unique_ptr<DepthStencilBuffer> CreateDefaultDepthStencilBuffer(
  ID3D12Device* device,
  int width,
  int height,
  D3D12_CPU_DESCRIPTOR_HANDLE outCPUHandle
);

struct RenderTarget2D {
  std::unique_ptr<Texture2D> texture;
  std::unique_ptr<RenderTargetView> rtv;
  std::unique_ptr<ShaderResourceView> srv;
  std::unique_ptr<UnorderedAccessView> uav;
};


}  // namespace dxh

