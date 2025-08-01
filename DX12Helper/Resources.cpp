#include "Resources.h"

#include <iostream>


static int g_resourceID = 0;

namespace
{

std::string ResourceStateString(D3D12_RESOURCE_STATES state)
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
    default:
      return std::to_string(state);
  }
}

void DebugPrintResourceStateTransition(
  const dxh::RawResource* resource,
  D3D12_RESOURCE_STATES before,
  D3D12_RESOURCE_STATES after
)
{
  if (resource->ShouldDebug()) {
    std::cout << "[ " << resource->Name() << ": " << ResourceStateString(before) << " -> "
              << ResourceStateString(after) << "]\n";
  }
}

void DoTrackedTransition(
  ID3D12GraphicsCommandList* cmdList,
  dxh::RawResource* resource,
  D3D12_RESOURCE_STATES stateBefore,
  D3D12_RESOURCE_STATES stateAfter
)
{
  if (stateBefore == stateAfter) {
    return;
  }
  DebugPrintResourceStateTransition(resource, stateBefore, stateAfter);
  auto t = CD3DX12_RESOURCE_BARRIER::Transition(resource->Resource(), stateBefore, stateAfter);
  cmdList->ResourceBarrier(1, &t);
  resource->MakeTransition(stateAfter);
}

}  // namespace


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


std::unique_ptr<Texture2D>
CreateRenderTargetTexture2D(ID3D12Device* device, DXGI_FORMAT format, UINT64 width, UINT height)
{
  auto BlackClear = BlackClearValue(format);
  return std::make_unique<Texture2D>(
    device, D3D12_RESOURCE_STATE_RENDER_TARGET, &BlackClear, format, width, height, 1, 0, 1, 0,
    D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_TEXTURE_LAYOUT_UNKNOWN, 0
  );
}

std::unique_ptr<Texture2D>
CreateDepthStencilTexture2D(ID3D12Device* device, DXGI_FORMAT format, UINT64 width, UINT height)
{
  CD3DX12_CLEAR_VALUE clearValue = DefaultDepthStencilClearValue(format);
  return std::make_unique<Texture2D>(
    device, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValue, format, width, height, 1, 0, 1, 0,
    D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL, D3D12_TEXTURE_LAYOUT_UNKNOWN, 0
  );
}

std::unique_ptr<DepthStencilBuffer> CreateDefaultDepthStencilBuffer(
  ID3D12Device* device,
  int width,
  int height,
  D3D12_CPU_DESCRIPTOR_HANDLE outCPUHandle
)
{
  auto format = DXGI_FORMAT_D24_UNORM_S8_UINT;
  auto clearValue = DefaultDepthStencilClearValue(format);
  auto tex = CreateDepthStencilTexture2D(device, format, width, height);
  auto dsv = CreateDefaultDSVForTexture2D(device, tex->Resource(), format, outCPUHandle);
  return std::make_unique<DepthStencilBuffer>(std::move(tex), std::move(dsv));
}

RawResource::RawResource(
  ID3D12Device* device,
  const D3D12_RESOURCE_DESC& desc,
  D3D12_RESOURCE_STATES state,
  const D3D12_CLEAR_VALUE* clearValue,
  D3D12_HEAP_TYPE heapType,
  D3D12_HEAP_FLAGS heapFlag
)
    : state_{state}
{
  auto heapProp = CD3DX12_HEAP_PROPERTIES(heapType);
  DX::ThrowIfFailed(device->CreateCommittedResource(
    &heapProp, heapFlag, &desc, state, clearValue, IID_PPV_ARGS(resource_.GetAddressOf())
  ));
  id_ = g_resourceID++;
}


}  // namespace dxh