#pragma once

#include <array>

#include "PCH.h"

namespace dxh
{

template<typename VertexType, typename IndexType>
struct TriangleMeshData {
  std::vector<VertexType> vertices;
  std::vector<IndexType> indices;

  TriangleMeshData() = default;

  template<typename IT, typename = std::enable_if_t<sizeof(IT) <= sizeof(IndexType)>>
  TriangleMeshData<VertexType, IndexType>(const TriangleMeshData<VertexType, IT>& other)
  {
    vertices = other.vertices;
    indices.resize(other.indices.size());
    std::copy(other.indices.begin(), other.indices.end(), indices.begin());
  }

  size_t TriangleCount() const { return indices.size() / 3; }
  size_t VertexCount() const { return vertices.size(); }
  size_t IndexCount() const { return indices.size(); }
  size_t VertexBufferByteSize() const { return vertices.size() * sizeof(VertexType); }
  size_t IndexBufferByteSize() const { return indices.size() * sizeof(IndexType); }
};


template<
  typename VT,
  typename IT1,
  typename IT2,
  typename IT = std::conditional_t<sizeof(IT1) < sizeof(IT2), IT2, IT1>>
TriangleMeshData<VT, IT>
MergeMeshData(const TriangleMeshData<VT, IT1>& mesh1, const TriangleMeshData<VT, IT2>& mesh2)
{
  TriangleMeshData<VT, IT> res = mesh1;

  return res;
}

}  // namespace dxh