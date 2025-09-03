#pragma once

#include <array>

#include "PCH.h"

namespace dxh
{

template<typename VertexType, typename IndexType>
struct TriangleMeshData {
  std::vector<VertexType> vertices;
  std::vector<IndexType> indices;

  size_t TriangleCount() const { return indices.size() / 3; }
  size_t VertexCount() const { return vertices.size(); }
  size_t IndexCount() const { return indices.size(); }
  size_t VertexBufferByteSize() const { return vertices.size() * sizeof(VertexType); }
  size_t IndexBufferByteSize() const { return indices.size() * sizeof(IndexType); }
};


}  // namespace dxh