
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
    // Front face (z = 0.5): v4, v5, v6, v7 - CCW: v4->v5->v6, v4->v6->v7
    4, 5, 6, 4, 6, 7,
    // Back face (z = -0.5): v0, v1, v2, v3 - CCW when viewed from outside: v0->v3->v2, v0->v2->v1
    0, 3, 2, 0, 2, 1,
    // Right face (x = 0.5): v1, v5, v6, v2 - CCW: v1->v2->v6, v1->v6->v5
    1, 2, 6, 1, 6, 5,
    // Left face (x = -0.5): v4, v0, v3, v7 - CCW: v4->v7->v3, v4->v3->v0
    4, 7, 3, 4, 3, 0,
    // Top face (y = 0.5): v3, v2, v6, v7 - CCW: v3->v7->v6, v3->v6->v2
    3, 7, 6, 3, 6, 2,
    // Bottom face (y = -0.5): v0, v1, v5, v4 - CCW: v0->v1->v5, v0->v5->v4
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

template<typename VertexType, typename IndexType>
TriangleMeshData<VertexType, IndexType> CreateUnitBoxWithNormal()
{
  TriangleMeshData<VertexType, IndexType> meshData;

  // 24 vertices (4 per face × 6 faces)
  meshData.vertices.resize(24);

  // Front face (z = +0.5, normal = +Z)
  meshData.vertices[0].position = {-0.5f, -0.5f, 0.5f};  // bottom-left (v4)
  meshData.vertices[0].normal = {0.0f, 0.0f, 1.0f};
  meshData.vertices[0].color = {255, 0, 255, 255};  // v4 color

  meshData.vertices[1].position = {0.5f, -0.5f, 0.5f};  // bottom-right (v5)
  meshData.vertices[1].normal = {0.0f, 0.0f, 1.0f};
  meshData.vertices[1].color = {0, 255, 255, 255};  // v5 color

  meshData.vertices[2].position = {0.5f, 0.5f, 0.5f};  // top-right (v6)
  meshData.vertices[2].normal = {0.0f, 0.0f, 1.0f};
  meshData.vertices[2].color = {255, 255, 255, 255};  // v6 color

  meshData.vertices[3].position = {-0.5f, 0.5f, 0.5f};  // top-left (v7)
  meshData.vertices[3].normal = {0.0f, 0.0f, 1.0f};
  meshData.vertices[3].color = {0, 0, 0, 255};  // v7 color

  // Back face (z = -0.5, normal = -Z)
  meshData.vertices[4].position = {0.5f, -0.5f, -0.5f};  // bottom-left (v1)
  meshData.vertices[4].normal = {0.0f, 0.0f, -1.0f};
  meshData.vertices[4].color = {0, 255, 0, 255};  // v1 color

  meshData.vertices[5].position = {-0.5f, -0.5f, -0.5f};  // bottom-right (v0)
  meshData.vertices[5].normal = {0.0f, 0.0f, -1.0f};
  meshData.vertices[5].color = {255, 0, 0, 255};  // v0 color

  meshData.vertices[6].position = {-0.5f, 0.5f, -0.5f};  // top-right (v3)
  meshData.vertices[6].normal = {0.0f, 0.0f, -1.0f};
  meshData.vertices[6].color = {255, 255, 0, 255};  // v3 color

  meshData.vertices[7].position = {0.5f, 0.5f, -0.5f};  // top-left (v2)
  meshData.vertices[7].normal = {0.0f, 0.0f, -1.0f};
  meshData.vertices[7].color = {0, 0, 255, 255};  // v2 color

  // Right face (x = +0.5, normal = +X)
  meshData.vertices[8].position = {0.5f, -0.5f, 0.5f};  // bottom-left (v5)
  meshData.vertices[8].normal = {1.0f, 0.0f, 0.0f};
  meshData.vertices[8].color = {0, 255, 255, 255};  // v5 color

  meshData.vertices[9].position = {0.5f, -0.5f, -0.5f};  // bottom-right (v1)
  meshData.vertices[9].normal = {1.0f, 0.0f, 0.0f};
  meshData.vertices[9].color = {0, 255, 0, 255};  // v1 color

  meshData.vertices[10].position = {0.5f, 0.5f, -0.5f};  // top-right (v2)
  meshData.vertices[10].normal = {1.0f, 0.0f, 0.0f};
  meshData.vertices[10].color = {0, 0, 255, 255};  // v2 color

  meshData.vertices[11].position = {0.5f, 0.5f, 0.5f};  // top-left (v6)
  meshData.vertices[11].normal = {1.0f, 0.0f, 0.0f};
  meshData.vertices[11].color = {255, 255, 255, 255};  // v6 color

  // Left face (x = -0.5, normal = -X)
  meshData.vertices[12].position = {-0.5f, -0.5f, -0.5f};  // bottom-left (v0)
  meshData.vertices[12].normal = {-1.0f, 0.0f, 0.0f};
  meshData.vertices[12].color = {255, 0, 0, 255};  // v0 color

  meshData.vertices[13].position = {-0.5f, -0.5f, 0.5f};  // bottom-right (v4)
  meshData.vertices[13].normal = {-1.0f, 0.0f, 0.0f};
  meshData.vertices[13].color = {255, 0, 255, 255};  // v4 color

  meshData.vertices[14].position = {-0.5f, 0.5f, 0.5f};  // top-right (v7)
  meshData.vertices[14].normal = {-1.0f, 0.0f, 0.0f};
  meshData.vertices[14].color = {0, 0, 0, 255};  // v7 color

  meshData.vertices[15].position = {-0.5f, 0.5f, -0.5f};  // top-left (v3)
  meshData.vertices[15].normal = {-1.0f, 0.0f, 0.0f};
  meshData.vertices[15].color = {255, 255, 0, 255};  // v3 color

  // Top face (y = +0.5, normal = +Y)
  meshData.vertices[16].position = {-0.5f, 0.5f, 0.5f};  // bottom-left (v7)
  meshData.vertices[16].normal = {0.0f, 1.0f, 0.0f};
  meshData.vertices[16].color = {0, 0, 0, 255};  // v7 color

  meshData.vertices[17].position = {0.5f, 0.5f, 0.5f};  // bottom-right (v6)
  meshData.vertices[17].normal = {0.0f, 1.0f, 0.0f};
  meshData.vertices[17].color = {255, 255, 255, 255};  // v6 color

  meshData.vertices[18].position = {0.5f, 0.5f, -0.5f};  // top-right (v2)
  meshData.vertices[18].normal = {0.0f, 1.0f, 0.0f};
  meshData.vertices[18].color = {0, 0, 255, 255};  // v2 color

  meshData.vertices[19].position = {-0.5f, 0.5f, -0.5f};  // top-left (v3)
  meshData.vertices[19].normal = {0.0f, 1.0f, 0.0f};
  meshData.vertices[19].color = {255, 255, 0, 255};  // v3 color

  // Bottom face (y = -0.5, normal = -Y)
  meshData.vertices[20].position = {-0.5f, -0.5f, -0.5f};  // bottom-left (v0)
  meshData.vertices[20].normal = {0.0f, -1.0f, 0.0f};
  meshData.vertices[20].color = {255, 0, 0, 255};  // v0 color

  meshData.vertices[21].position = {0.5f, -0.5f, -0.5f};  // bottom-right (v1)
  meshData.vertices[21].normal = {0.0f, -1.0f, 0.0f};
  meshData.vertices[21].color = {0, 255, 0, 255};  // v1 color

  meshData.vertices[22].position = {0.5f, -0.5f, 0.5f};  // top-right (v5)
  meshData.vertices[22].normal = {0.0f, -1.0f, 0.0f};
  meshData.vertices[22].color = {0, 255, 255, 255};  // v5 color

  meshData.vertices[23].position = {-0.5f, -0.5f, 0.5f};  // top-left (v4)
  meshData.vertices[23].normal = {0.0f, -1.0f, 0.0f};
  meshData.vertices[23].color = {255, 0, 255, 255};  // v4 color

  // 36 indices (6 faces × 2 triangles × 3 indices)
  meshData.indices = {
    // Front face (CCW)
    0, 1, 2, 0, 2, 3,
    // Back face (CCW)
    4, 5, 6, 4, 6, 7,
    // Right face (CCW)
    8, 9, 10, 8, 10, 11,
    // Left face (CCW)
    12, 13, 14, 12, 14, 15,
    // Top face (CCW)
    16, 17, 18, 16, 18, 19,
    // Bottom face (CCW)
    20, 21, 22, 20, 22, 23
  };

  return meshData;
}

}  // namespace dxh