#pragma once

#include <stdexcept>

#include "CommandObjects.h"
#include "DrawCommands.h"
#include "Fence.h"
#include "Geometry.h"
#include "Resources.h"

namespace dxh
{

template<typename VertexType, typename IndexType>
struct VertexBuffer {
  std::unique_ptr<DefaultHeapBuffer> buffer;

  VertexBuffer(
    ID3D12Device* device,
    GraphicsCommandList& cmdList,
    CommandAllocator& cmdAlloc,
    CommandQueue& cmdQueue,
    Fence& fence,
    const IFaceVertexTriangleMesh<VertexType, IndexType>& mesh
  )
  {
    auto byteSize = VertexArrayByteSize(mesh);
    buffer = std::make_unique<DefaultHeapBuffer>(device, byteSize);

    auto vertexCount = mesh.VertexCount();
    auto uploader = UploadHeapArray<VertexType>{device, vertexCount};

    auto vertices = AssembleVertexArray(mesh);
    uploader.LoadBuffer(vertices.data(), byteSize);

    // Send to GPU
    ResetCommandAllocator(cmdAlloc);
    ResetCommandList(cmdList, cmdAlloc);

    auto initCmds = CopyBuffer(buffer.get(), 0, &uploader, 0, uploader.BufferByteSize());
    cmdList.RecordDrawCommands(initCmds);

    ExecuteQueuedCommands(cmdList, cmdQueue);
    fence.WaitForGPUCompletion(cmdQueue.Get());
  }


  D3D12_VERTEX_BUFFER_VIEW GetVBV() const
  {
    D3D12_VERTEX_BUFFER_VIEW vbv{};
    vbv.BufferLocation = buffer->GPUVirtualAddress();
    vbv.SizeInBytes = buffer->BufferByteSize();
    vbv.StrideInBytes = sizeof(VertexType);
    return vbv;
  }
};

using DefaultVertexBuffer = VertexBuffer<DefaultVertex, DefaultIndex>;


template<typename VertexType, typename IndexType>
struct IndexBuffer {
  std::unique_ptr<DefaultHeapBuffer> buffer;

  IndexBuffer(
    ID3D12Device* device,
    GraphicsCommandList& cmdList,
    CommandAllocator& cmdAlloc,
    CommandQueue& cmdQueue,
    Fence& fence,
    const IFaceVertexTriangleMesh<VertexType, IndexType>& mesh
  )
  {
    auto byteSize = IndexArrayByteSize(mesh);
    buffer = std::make_unique<DefaultHeapBuffer>(device, byteSize);

    auto indexCount = 3 * mesh.FaceCount();
    auto uploader = UploadHeapArray<IndexType>{device, indexCount};

    auto indices = AssembleIndexArray(mesh);
    uploader.LoadBuffer(indices.data(), byteSize);

    ResetCommandAllocator(cmdAlloc);
    ResetCommandList(cmdList, cmdAlloc);

    auto initCmds = CopyBuffer(buffer.get(), 0, &uploader, 0, uploader.BufferByteSize());
    cmdList.RecordDrawCommands(initCmds);

    ExecuteQueuedCommands(cmdList, cmdQueue);
    fence.WaitForGPUCompletion(cmdQueue.Get());
  }

  D3D12_INDEX_BUFFER_VIEW GetIBV() const
  {
    D3D12_INDEX_BUFFER_VIEW ibv{};
    ibv.BufferLocation = buffer->GPUVirtualAddress();
    ibv.SizeInBytes = buffer->BufferByteSize();
    ibv.Format = IndexType::format;
    return ibv;
  }
};

using DefaultIndexBuffer = IndexBuffer<DefaultVertex, DefaultIndex>;


}  // namespace dxh
