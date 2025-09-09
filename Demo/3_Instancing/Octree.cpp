
#include "Octree.h"

using namespace DirectX;

bool ShouldSubdivide(float boxSize, size_t objectCount, int depth)
{
  return boxSize > 4.f && objectCount > 4 && depth < 8;
}

std::vector<AABB> SubdivideBox(const AABB& box)
{
  std::vector<AABB> res(8);

  float dx = (box.max.x - box.min.x) / 2.0f;
  float dy = (box.max.y - box.min.y) / 2.0f;
  float dz = (box.max.z - box.min.z) / 2.0f;

  XMVECTOR xmMin = XMVectorSet(box.min.x, box.min.y, box.min.z, 0.f);

  XMVECTOR newMin[8] = {
    xmMin,
    XMVectorAdd(xmMin, XMVectorSet(dx, 0.f, 0.f, 0.f)),
    XMVectorAdd(xmMin, XMVectorSet(0.f, dy, 0.f, 0.f)),
    XMVectorAdd(xmMin, XMVectorSet(dx, dy, 0.f, 0.f)),
    XMVectorAdd(xmMin, XMVectorSet(0.f, 0.f, dz, 0.f)),
    XMVectorAdd(xmMin, XMVectorSet(dx, 0.f, dz, 0.f)),
    XMVectorAdd(xmMin, XMVectorSet(0.f, dy, dz, 0.f)),
    XMVectorAdd(xmMin, XMVectorSet(dx, dy, dz, 0.f))
  };

  XMVECTOR newMax[8];
  for (int i = 0; i < 8; ++i) {
    newMax[i] = XMVectorAdd(newMin[i], XMVectorSet(dx, dy, dz, 0.f));

    XMFLOAT3 pmin;
    XMStoreFloat3(&pmin, newMin[i]);
    XMFLOAT3 pmax;
    XMStoreFloat3(&pmax, newMax[i]);

    res[i] = {pmin, pmax};
    assert(res[i].IsValid());
  }

  return res;
}