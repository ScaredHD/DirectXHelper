#include "Resources.h"

#include <iostream>


namespace dxh
{
CD3DX12_CLEAR_VALUE ZeroClearValue(DXGI_FORMAT format)
{
  FLOAT val[] = {0.f, 0.f, 0.f, 0.f};
  return {format, val};
}

CD3DX12_CLEAR_VALUE BlackClearValue(DXGI_FORMAT format)
{
  return RGBAClearValue(format, 0.f, 0.f, 0.f, 1.f);
}

CD3DX12_CLEAR_VALUE RGBAClearValue(DXGI_FORMAT format, float r, float g, float b, float a)
{
  FLOAT val[] = {r, g, b, a};
  return {format, val};
}

CD3DX12_CLEAR_VALUE DefaultDepthStencilClearValue(DXGI_FORMAT format)
{
  return {format, 1.f, 0};
}

int ComputePaddedSize(int size, int alignment)
{
  if (alignment <= 0) {
    throw std::runtime_error{"alignment must be positive"};
  }
  int padding = (alignment - size % alignment) % alignment;
  return size + padding;
}

std::string ResourceStateToString(D3D12_RESOURCE_STATES state)
{
  switch (state) {
    case D3D12_RESOURCE_STATE_COMMON:
      return "COMMON|PRESENT";
    case D3D12_RESOURCE_STATE_RENDER_TARGET:
      return "RENDER_TARGET";
    case D3D12_RESOURCE_STATE_DEPTH_WRITE:
      return "DEPTH_WRITE";
    case D3D12_RESOURCE_STATE_DEPTH_READ:
      return "DEPTH_READ";
    case D3D12_RESOURCE_STATE_GENERIC_READ:
      return "GENERIC_READ";
    case D3D12_RESOURCE_STATE_COPY_SOURCE:
      return "COPY_SOURCE";
    case D3D12_RESOURCE_STATE_COPY_DEST:
      return "COPY_DEST";
    case D3D12_RESOURCE_STATE_UNORDERED_ACCESS:
      return "UNORDERED_ACCESS";
    case D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER:
      return "VERTEX_AND_CONSTANT_BUFFER";
    case D3D12_RESOURCE_STATE_INDEX_BUFFER:
      return "INDEX_BUFFER";
    case D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE:
      return "NON_PIXEL_SHADER_RESOURCE";
    case D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE:
      return "PIXEL_SHADER_RESOURCE";
    case D3D12_RESOURCE_STATE_STREAM_OUT:
      return "STREAM_OUT";
    case D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT:
      return "INDIRECT_ARGUMENT";
    case D3D12_RESOURCE_STATE_RESOLVE_SOURCE:
      return "RESOLVE_SOURCE";
    case D3D12_RESOURCE_STATE_RESOLVE_DEST:
      return "RESOLVE_DEST";
    case D3D12_RESOURCE_STATE_VIDEO_DECODE_READ:
      return "VIDEO_DECODE_READ";
    case D3D12_RESOURCE_STATE_VIDEO_DECODE_WRITE:
      return "VIDEO_DECODE_WRITE";
    case D3D12_RESOURCE_STATE_VIDEO_PROCESS_READ:
      return "VIDEO_PROCESS_READ";
    case D3D12_RESOURCE_STATE_VIDEO_PROCESS_WRITE:
      return "VIDEO_PROCESS_WRITE";
    case D3D12_RESOURCE_STATE_VIDEO_ENCODE_READ:
      return "VIDEO_ENCODE_READ";
    case D3D12_RESOURCE_STATE_VIDEO_ENCODE_WRITE:
      return "VIDEO_ENCODE_WRITE";
    default:
      return std::to_string(state);
  }
}

}  // namespace dxh