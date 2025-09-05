#include "Culling.h"

using namespace DirectX;

#include <cmath>

bool IntersectAABBPlane(const AABB& box, const Plane& plane)
{
  const auto& [a, b, c] = plane.Normal();
  const auto& pmin = box.min;
  const auto& pmax = box.max;

  XMFLOAT3 g;  // closest point to plane
  g.x = (a > 0) ? pmax.x : pmin.x;
  g.y = (b > 0) ? pmax.y : pmin.y;
  g.z = (c > 0) ? pmax.z : pmin.z;

  float sign = plane(g);
  return sign > 0;
}

Plane TransformPlane(const XMMATRIX& mat, const Plane& plane)
{
  XMMATRIX xmT = XMMatrixTranspose(XMMatrixInverse(nullptr, mat));
  XMVECTOR xmPlane = XMVectorSet(plane.a, plane.b, plane.c, plane.d);
  XMVECTOR xmTransformedPlane = XMVector4Transform(xmPlane, xmT);
  XMFLOAT4 p;
  XMStoreFloat4(&p, xmTransformedPlane);
  return Plane{p.x, p.y, p.z, p.w};
}