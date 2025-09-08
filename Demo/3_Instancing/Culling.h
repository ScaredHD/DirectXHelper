#pragma once

#include <DirectXMath.h>

struct AABB {
  DirectX::XMFLOAT3 min;
  DirectX::XMFLOAT3 max;
};

AABB TransformAABB(const DirectX::XMMATRIX& mat, const AABB& box);

struct Plane {
  float a;
  float b;
  float c;
  float d;

  DirectX::XMFLOAT3 Normal() const { return {a, b, c}; }
  float operator()(const DirectX::XMFLOAT3& point) const
  {
    return a * point.x + b * point.y + c * point.z + d;
  }
};

struct Frustum {
  Plane planes[6];
};

bool IntersectAABBPlane(const AABB& box, const Plane& plane);

Plane TransformPlane(const DirectX::XMMATRIX& mat, const Plane& plane);