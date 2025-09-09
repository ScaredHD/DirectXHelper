#pragma once

#include <DirectXMath.h>

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

struct AABB {
  DirectX::XMFLOAT3 min = {0.f , 0.f, 0.f};
  DirectX::XMFLOAT3 max = {0.f , 0.f, 0.f};

  bool IsValid() const { return min.x <= max.x && min.y <= max.y && min.z <= max.z; }

  bool Contains(const DirectX::XMFLOAT3& point) const;
  bool Contains(const AABB& box) const;
  bool Intersect(const Plane& plane) const;
  bool Intersect(const Frustum& frustum) const;
};


void GetAABBPoints(const AABB& box, DirectX::XMVECTOR points[8]);

AABB TransformAABB(const DirectX::XMMATRIX& mat, const AABB& box);

bool IntersectAABBPlane(const AABB& box, const Plane& plane);

Plane TransformPlane(const DirectX::XMMATRIX& mat, const Plane& plane);