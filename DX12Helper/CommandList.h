#pragma once

#include "PCH.h"
#include "Geometry/GeometryRender.h"


namespace dxh
{

class TrackedResource;

template<size_t N>
class SwapChain;

template<typename VertexType, typename IndexType>
class TriangleMeshRenderResource;

class GraphicsCommandList
{
public:
  explicit GraphicsCommandList(ID3D12Device* device, ID3D12CommandAllocator* alloc);

  ID3D12GraphicsCommandList* Get() const;

  void Reset(class CommandAllocator& alloc);

  void Reset(ID3D12CommandAllocator* alloc);

  void Close() const;

  void Execute(ID3D12CommandQueue* cmdQueue) const;

  void Execute(class CommandQueue& cmdQueue) const;

  void Transition(
    ID3D12Resource* resource,
    D3D12_RESOURCE_STATES stateBefore,
    D3D12_RESOURCE_STATES stateAfter
  ) const;

  void Transition(TrackedResource& resource, D3D12_RESOURCE_STATES targetState);

  void SetRootSignature(const class RootSignature& rootSignature);

  void SetPipelineState(ID3D12PipelineState* pso);

  void SetRootCBV(UINT rootParameterIndex, ID3D12Resource* resource);

  void SetViewport(const SwapChain<2>& swapChain) const;

  void SetScissorRect(const SwapChain<2>& swapChain) const;

  void SetRenderTargets(UINT count, D3D12_CPU_DESCRIPTOR_HANDLE renderTargets[]);

  void ClearRTV(D3D12_CPU_DESCRIPTOR_HANDLE rtv, std::array<float, 4> color) const;

  void SetVBV(D3D12_VERTEX_BUFFER_VIEW vbv);

  void SetIBV(D3D12_INDEX_BUFFER_VIEW ibv);

  template<typename VertexType, typename IndexType>
  void SetTriangleMeshToDraw(const TriangleMeshRenderResource<VertexType, IndexType>& meshResource);

private:
  Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList;
};

template<typename VertexType, typename IndexType>
void GraphicsCommandList::SetTriangleMeshToDraw(
  const TriangleMeshRenderResource<VertexType, IndexType>& meshResource
)
{
  SetVBV(meshResource.VBV());
  SetIBV(meshResource.IBV());

  Get()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}


}  // namespace dxh