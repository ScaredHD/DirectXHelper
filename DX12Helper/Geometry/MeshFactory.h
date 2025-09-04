
#pragma once

#include "Geometry.h"


namespace dxh
{


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
    vertices[0], vertices[1], vertices[2], vertices[3],  // Back face (z = -0.5)
    vertices[4], vertices[5], vertices[6], vertices[7]   // Front face (z = 0.5)
  };
  meshData.indices = {
    // Front face (z = 0.5): v4, v5, v6, v7 - normal points +Z (outward)
    4, 5, 6, 4, 6, 7,
    // Back face (z = -0.5): v0, v1, v2, v3 - normal points -Z (outward)
    0, 1, 2, 0, 2, 3,
    // Right face (x = 0.5): v1, v5, v6, v2 - normal points +X (outward)
    1, 5, 6, 1, 6, 2,
    // Left face (x = -0.5): v4, v0, v3, v7 - normal points -X (outward)
    4, 0, 3, 4, 3, 7,
    // Top face (y = 0.5): v3, v2, v6, v7 - normal points +Y (outward)
    3, 2, 6, 3, 6, 7,
    // Bottom face (y = -0.5): v0, v1, v5, v4 - normal points -Y (outward)
    0, 1, 5, 0, 5, 4
  };
  return meshData;
}

template<typename VertexType, typename IndexType>
TriangleMeshData<VertexType, IndexType> CreateUnitBox()
{
  VertexType v0, v1, v2, v3, v4, v5, v6, v7;
  v0.position = {-0.5f, -0.5f, -0.5f};
  v0.color = {255, 0, 0, 255};
  v1.position = {0.5f, -0.5f, -0.5f};
  v1.color = {0, 255, 0, 255};
  v2.position = {0.5f, 0.5f, -0.5f};
  v2.color = {0, 0, 255, 255};
  v3.position = {-0.5f, 0.5f, -0.5f};
  v3.color = {255, 255, 0, 255};
  v4.position = {-0.5f, -0.5f, 0.5f};
  v4.color = {255, 0, 255, 255};
  v5.position = {0.5f, -0.5f, 0.5f};
  v5.color = {0, 255, 255, 255};
  v6.position = {0.5f, 0.5f, 0.5f};
  v6.color = {255, 255, 255, 255};
  v7.position = {-0.5f, 0.5f, 0.5f};
  v7.color = {0, 0, 0, 255};
  std::array<VertexType, 8> vertices = {v0, v1, v2, v3, v4, v5, v6, v7};
  return CreateBox<VertexType, IndexType>(vertices);
}

}  // namespace dxh