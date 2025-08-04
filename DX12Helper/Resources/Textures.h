#pragma once

#include "Resources.h"

namespace dxh
{


class Texture : public TrackedResource
{
public:
  explicit Texture(
    ID3D12Device* device,
    D3D12_RESOURCE_FLAGS resFlags,
    D3D12_RESOURCE_DIMENSION dimension,
    DXGI_FORMAT format,
    UINT64 width,
    UINT height,
    UINT16 depthOrArraySize,
    UINT16 mipLevels = 0,
    const D3D12_CLEAR_VALUE* clearValue = nullptr,
    D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON,
    UINT sampleCount = 1,
    UINT sampleQuality = 0,
    D3D12_TEXTURE_LAYOUT texLayout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
    UINT64 alignment = 0
  )
      : TrackedResource{device, CD3DX12_RESOURCE_DESC{dimension, alignment, width, height, depthOrArraySize, mipLevels, format, sampleCount, sampleQuality, texLayout, resFlags}, state, clearValue, D3D12_HEAP_TYPE_DEFAULT, D3D12_HEAP_FLAG_NONE},
        format{format},
        width{width},
        height{height},
        depthOrArraySize{depthOrArraySize},
        mipLevels{mipLevels}
  {
  }

  DXGI_FORMAT Format() const { return format; }
  UINT64 Width() const { return width; }
  UINT Height() const { return height; }
  UINT16 DepthOrArraySize() const { return depthOrArraySize; }
  UINT16 MipLevels() const { return mipLevels; }

private:
  DXGI_FORMAT format;
  UINT64 width;
  UINT height;
  UINT16 depthOrArraySize;
  UINT16 mipLevels;
};

class Texture2D : public Texture
{
public:
  explicit Texture2D(
    ID3D12Device* device,
    D3D12_RESOURCE_FLAGS resFlags,
    DXGI_FORMAT format,
    UINT64 width,
    UINT height,
    UINT16 mipLevels = 0,
    const D3D12_CLEAR_VALUE* clearValue = nullptr,
    D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON,
    UINT sampleCount = 1,
    UINT sampleQuality = 0,
    D3D12_TEXTURE_LAYOUT texLayout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
    UINT64 alignment = 0
  )
      : Texture{device,      resFlags,      D3D12_RESOURCE_DIMENSION_TEXTURE2D,
                format,      width,         height,
                1,  // depth or array size
                mipLevels,   clearValue,    state,
                sampleCount, sampleQuality, texLayout,
                alignment}
  {
  }
};

class Texture3D : public Texture
{
  explicit Texture3D(
    ID3D12Device* device,
    D3D12_RESOURCE_FLAGS resFlags,
    DXGI_FORMAT format,
    UINT64 width,
    UINT height,
    UINT16 depthOrArraySize,
    UINT16 mipLevels = 0,
    const D3D12_CLEAR_VALUE* clearValue = nullptr,
    D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON,
    UINT sampleCount = 1,
    UINT sampleQuality = 0,
    D3D12_TEXTURE_LAYOUT texLayout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
    UINT64 alignment = 0
  )
      : Texture{
          device,
          resFlags,
          D3D12_RESOURCE_DIMENSION_TEXTURE3D,
          format,
          width,
          height,
          depthOrArraySize,
          mipLevels,
          clearValue,
          state,
          sampleCount,
          sampleQuality,
          texLayout,
          alignment
        }
  {
  }

  UINT16 Depth() const { return DepthOrArraySize(); }
};

}  // namespace dxh