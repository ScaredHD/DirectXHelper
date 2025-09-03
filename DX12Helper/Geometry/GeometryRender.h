#pragma once

#include <memory>

#include "Buffers.h"
#include "Geometry.h"


namespace dxh
{

template<typename VertexType>
D3D12_VERTEX_BUFFER_VIEW
CreateVertexBufferView(D3D12_GPU_VIRTUAL_ADDRESS bufferLocation, size_t vertexCount)
{
  D3D12_VERTEX_BUFFER_VIEW vbView{};
  vbView.BufferLocation = bufferLocation;
  vbView.SizeInBytes = static_cast<UINT>(vertexCount * sizeof(VertexType));
  vbView.StrideInBytes = sizeof(VertexType);
  return vbView;
}

template<size_t byteSize>
struct ByteSizeToIndexFormat;

template<>
struct ByteSizeToIndexFormat<1> {
  static constexpr DXGI_FORMAT format = DXGI_FORMAT_R8_UINT;
};

template<>
struct ByteSizeToIndexFormat<2> {
  static constexpr DXGI_FORMAT format = DXGI_FORMAT_R16_UINT;
};

template<>
struct ByteSizeToIndexFormat<4> {
  static constexpr DXGI_FORMAT format = DXGI_FORMAT_R32_UINT;
};

template<typename IndexType>
struct IndexFormat {
  static constexpr DXGI_FORMAT format = ByteSizeToIndexFormat<sizeof(IndexType)>::format;
};


template<typename IndexType>
D3D12_INDEX_BUFFER_VIEW
CreateIndexBufferView(D3D12_GPU_VIRTUAL_ADDRESS bufferLocation, size_t indexCount)
{
  D3D12_INDEX_BUFFER_VIEW ibView{};
  ibView.BufferLocation = bufferLocation;
  ibView.SizeInBytes = static_cast<UINT>(indexCount * sizeof(IndexType));
  ibView.Format = IndexFormat<IndexType>::format;
  return ibView;
}


template<typename VertexType, typename IndexType>
class TriangleMeshRenderResource
{
public:
  explicit TriangleMeshRenderResource(
    ID3D12Device* device,
    const TriangleMeshData<VertexType, IndexType>* meshData
  )
      : meshData(meshData)
  {
    StageMeshData(device, meshData);
  }


  D3D12_VERTEX_BUFFER_VIEW VBV() const { return vbv; }
  D3D12_INDEX_BUFFER_VIEW IBV() const { return ibv; }

  void StageMeshData(ID3D12Device* device, const TriangleMeshData<VertexType, IndexType>* meshData)
  {
    if (!meshData) {
      return;
    }

    this->meshData = meshData;

    vertexBuffer = std::make_unique<DefaultHeapBuffer>(device, meshData->VertexBufferByteSize());
    vertexBuffer->StageUpload(0, meshData->vertices.data(), 0, meshData->VertexBufferByteSize());

    indexBuffer = std::make_unique<DefaultHeapBuffer>(device, meshData->IndexBufferByteSize());
    indexBuffer->StageUpload(0, meshData->indices.data(), 0, meshData->IndexBufferByteSize());

    vbv = CreateVertexBufferView<VertexType>(
      vertexBuffer->Resource()->GetGPUVirtualAddress(), meshData->VertexCount()
    );

    ibv = CreateIndexBufferView<IndexType>(
      indexBuffer->Resource()->GetGPUVirtualAddress(), meshData->IndexCount()
    );
  }

  void QueueUploadMeshData(GraphicsCommandList& cmdList)
  {
    vertexBuffer->QueueUpload(cmdList);
    indexBuffer->QueueUpload(cmdList);
  }

private:
  const TriangleMeshData<VertexType, IndexType>* meshData;
  std::unique_ptr<DefaultHeapBuffer> vertexBuffer;
  std::unique_ptr<DefaultHeapBuffer> indexBuffer;
  D3D12_VERTEX_BUFFER_VIEW vbv;
  D3D12_INDEX_BUFFER_VIEW ibv;
};


}  // namespace dxh