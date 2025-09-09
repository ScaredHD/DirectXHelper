#include "Culling.h"

using namespace DirectX;

#include <cmath>

void GetAABBPoints(const AABB& box, XMVECTOR points[8])
{
  points[0] = XMVectorSet(box.min.x, box.min.y, box.min.z, 1.0f);
  points[1] = XMVectorSet(box.min.x, box.min.y, box.max.z, 1.0f);
  points[2] = XMVectorSet(box.min.x, box.max.y, box.min.z, 1.0f);
  points[3] = XMVectorSet(box.min.x, box.max.y, box.max.z, 1.0f);
  points[4] = XMVectorSet(box.max.x, box.min.y, box.min.z, 1.0f);
  points[5] = XMVectorSet(box.max.x, box.min.y, box.max.z, 1.0f);
  points[6] = XMVectorSet(box.max.x, box.max.y, box.min.z, 1.0f);
  points[7] = XMVectorSet(box.max.x, box.max.y, box.max.z, 1.0f);
}

AABB TransformAABB(const DirectX::XMMATRIX& mat, const AABB& box)
{
  XMVECTOR xmP[8];
  GetAABBPoints(box, xmP);

  for (auto& point : xmP) {
    point = XMVector3Transform(point, mat);
  }

  XMVECTOR xmMin = xmP[0];
  XMVECTOR xmMax = xmP[0];
  for (int i = 1; i < 8; ++i) {
    xmMin = XMVectorMin(xmMin, xmP[i]);
    xmMax = XMVectorMax(xmMax, xmP[i]);
  }

  XMFLOAT3 pmin;
  XMStoreFloat3(&pmin, xmMin);
  XMFLOAT3 pmax;
  XMStoreFloat3(&pmax, xmMax);

  return {pmin, pmax};
}

bool AABB::Contains(const AABB& box) const
{
  auto xmMin = XMLoadFloat3(&min);
  auto xmMax = XMLoadFloat3(&max);
  auto xmBoxMin = XMLoadFloat3(&box.min);
  auto xmBoxMax = XMLoadFloat3(&box.max);
  return XMVector3Less(xmMin, xmBoxMin) && XMVector3Less(xmBoxMax, xmMax);
}

bool AABB::Contains(const XMFLOAT3& point) const
{
  auto xmMin = XMLoadFloat3(&min);
  auto xmMax = XMLoadFloat3(&max);
  auto xmPoint = XMLoadFloat3(&point);
  return XMVector3Less(xmMin, xmPoint) && XMVector3Less(xmPoint, xmMax);
}

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


bool AABB::Intersect(const Plane& plane) const
{
  return IntersectAABBPlane(*this, plane);
}

bool AABB::Intersect(const Frustum& frustum) const
{
  for (const auto& plane : frustum.planes) {
    if (!IntersectAABBPlane(*this, plane)) {
      return false;
    }
  }
  return true;
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