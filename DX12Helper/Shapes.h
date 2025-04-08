#pragma once

#include "Geometry.h"

namespace dxh
{


template<typename VertexType, typename IndexType>
struct Triangle : public IFaceVertexTriangleMesh<VertexType, IndexType> {
  VertexType v0;
  VertexType v1;
  VertexType v2;

  FaceIndices GetFace(size_t idx) const override { return {0, 1, 2}; }

  size_t FaceCount() const override { return 1; }

  VertexType GetVertex(size_t idx) const override
  {
    switch (idx) {
      case 0:
        return v0;
      case 1:
        return v1;
      case 2:
        return v2;
      default:
        throw std::runtime_error{"error idx"};
        break;
    }
  }

  size_t VertexCount() const override { return 3; }
};

using DefaultTriangle = Triangle<DefaultVertex, DefaultIndex>;


}  // namespace dxh