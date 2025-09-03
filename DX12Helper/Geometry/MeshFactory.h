
#pragma once

#include "Geometry.h"


namespace dxh
{

template<typename VertexType, typename IndexType>
TriangleMeshData<VertexType, IndexType> MergeMeshData(
  const TriangleMeshData<VertexType, IndexType>& mesh1,
  const TriangleMeshData<VertexType, IndexType>& mesh2
)
{
  TriangleMeshData<VertexType, IndexType> res = mesh1;

  // TODO:

  return res;
}

template<typename VertexType, typename IndexType>
TriangleMeshData<VertexType, IndexType> CreateTriangle(VertexType v0, VertexType v1, VertexType v2)
{
  TriangleMeshData<VertexType, IndexType> meshData;
  meshData.vertices = {v0, v1, v2};
  meshData.indices = {0, 1, 2};
  return meshData;
}

template<typename VertexType, typename IndexType>
TriangleMeshData<VertexType, IndexType>
CreateQuad(VertexType v0, VertexType v1, VertexType v2, VertexType v3)
{
  TriangleMeshData<VertexType, IndexType> meshData;
  meshData.vertices = {v0, v1, v2, v3};
  meshData.indices = {0, 1, 2, 0, 2, 3};
  return meshData;
}

template<typename VertexType, typename IndexType>
TriangleMeshData<VertexType, IndexType> CreateBox(std::array<VertexType, 8> vertices)
{
  TriangleMeshData<VertexType, IndexType> meshData;
  meshData.vertices = {
    vertices[0], vertices[1], vertices[2], vertices[3],  // Front face
    vertices[4], vertices[5], vertices[6], vertices[7]   // Back face
  };
  meshData.indices = {
    0, 1, 2, 0, 2, 3,  // Front face
    4, 5, 6, 4, 6, 7,  // Back face
    0, 1, 5, 0, 5, 4,  // Left face
    2, 3, 7, 2, 7, 6   // Right face
  };
  return meshData;
}

}  // namespace dxh