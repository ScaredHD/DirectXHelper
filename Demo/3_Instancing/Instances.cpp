#include "Instances.h"

#include <iomanip>

#include "Culling.h"

using namespace DirectX;

size_t g_instanceCount = 1'000'000;

int GridSize()
{
  return static_cast<int>(std::sqrt(g_instanceCount));
}

float FieldSize()
{
  return 2.5f * static_cast<float>(GridSize());
}

float g_waveFreq = 0.5f;
float g_rotSpeed = 100.f;
float g_yOffsetAmplitude = 3.f;

int GridRowCount()
{
  return static_cast<int>(std::ceill(std::sqrt(g_instanceCount)));
}

int GridColCount()
{
  return GridSize();
}

using CoordI = std::pair<int, int>;
using CoordF = std::pair<float, float>;

std::pair<int, int> GridCoord(size_t index)
{
  int gridSize = GridSize();
  int row = static_cast<int>(index) / gridSize;
  int col = static_cast<int>(index) % gridSize;
  return {row, col};
}

std::pair<float, float> GridCoordNorm(std::pair<int, int> coord)
{
  auto [r, c] = coord;
  auto rowCount = GridRowCount();
  auto colCount = GridColCount();
  float u = static_cast<float>(c) / static_cast<float>(colCount - 1);
  float v = static_cast<float>(r) / static_cast<float>(rowCount - 1);
  return {u, v};
}

std::pair<float, float> GridCoordSNorm(std::pair<int, int> coord)
{
  auto [u, v] = GridCoordNorm(coord);
  u = u * 2.0f - 1.0f;
  v = v * 2.0f - 1.0f;
  return {u, v};
}

XMFLOAT3 InstanceBasePosition(size_t index)
{
  auto [u, v] = GridCoordSNorm(GridCoord(index));
  auto x = u * FieldSize() * 0.5f;
  auto z = v * FieldSize() * 0.5f;
  return {x, 0.0f, z};
}

float InstanceYOffset(size_t index, float time)
{
  auto [r, c] = GridCoord(index);
  return g_yOffsetAmplitude * std::sinf(g_waveFreq * (static_cast<float>(r) + time)) *
         std::cosf(g_waveFreq * (static_cast<float>(c) + time));
}

XMFLOAT3 RotationAxis(CoordF coordSNorm)
{
  auto [x, z] = coordSNorm;
  float dist = std::sqrt(x * x + z * z);
  XMFLOAT3 axis = {z, dist, -x};
  return axis;
}

XMMATRIX InstanceRotationMatrix(size_t index, float time = 0.f)
{
  auto coord = GridCoord(index);
  auto coordSNorm = GridCoordSNorm(coord);
  auto axis = RotationAxis(coordSNorm);
  float angle = std::fmod(time * g_rotSpeed, 360.f);
  float rad = XMConvertToRadians(angle);

  XMMATRIX rotMat = XMMatrixRotationAxis(XMLoadFloat3(&axis), rad);
  return rotMat;
}

XMFLOAT4X4 InstanceWorldMatrix(size_t index, float time = 0.f)
{
  auto pos = InstanceBasePosition(index);
  pos.y += InstanceYOffset(index, time);
  XMMATRIX t = XMMatrixTranslation(pos.x, pos.y, pos.z);
  XMMATRIX r = InstanceRotationMatrix(index, time);
  XMMATRIX world = r * t;
  XMFLOAT4X4 worldMat;
  XMStoreFloat4x4(&worldMat, world);
  return worldMat;
}

InstanceData GetInstance(size_t index, float time = 0.f)
{
  InstanceData instance;
  instance.world = InstanceWorldMatrix(index, time);

  XMMATRIX worldMat = XMLoadFloat4x4(&instance.world);
  XMMATRIX invWorldMat = XMMatrixInverse(nullptr, worldMat);
  XMStoreFloat4x4(&instance.invWorld, invWorldMat);

  float r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
  float g = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
  float b = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
  instance.albedo = {r, g, b, 1.0f};

  return instance;
}

std::vector<InstanceData> GetInstanceData(float time = 0.f)
{
  std::vector<InstanceData> instances;
  instances.reserve(g_instanceCount);
  for (size_t i = 0; i < g_instanceCount; ++i) {
    instances.push_back(GetInstance(i, time));
  }
  return instances;
}

std::vector<InstanceData> g_instanceBuffer = GetInstanceData();

void UpdateInstancePosition(float time)
{
  for (size_t i = 0; i < g_instanceCount; ++i) {
    g_instanceBuffer[i].world = InstanceWorldMatrix(i, time);
  }
}

std::vector<InstanceData> g_culledInstanceBuffer;
std::vector<size_t> g_culledInstanceIndices(g_instanceCount);
size_t g_cullCounter = 0;

Frustum CameraFrustumNDC()
{
  static Frustum f;
  f.planes[0] = {1, 0, 0, 1};   // left (1 x + 0 y + 0 z + 1 = 0) normal (1, 0, 0)
  f.planes[1] = {-1, 0, 0, 1};  // right
  f.planes[2] = {0, 1, 0, 1};   // bottom
  f.planes[3] = {0, -1, 0, 1};  // top
  f.planes[4] = {0, 0, 1, 0};   // near
  f.planes[5] = {0, 0, -1, 1};  // far
  return f;
}

void CullInstancesLocalSpace(const dxh::PerspectiveCamera& cam)
{
  g_cullCounter = 0;

  Frustum frustum = CameraFrustumNDC();
  AABB instanceAABB = {{-0.5f, -0.5f, -0.5f}, {0.5f, 0.5f, 0.5f}};

  XMFLOAT4X4 viewMatrix = cam.ViewMatrix();
  XMMATRIX xmView = XMLoadFloat4x4(&viewMatrix);

  XMFLOAT4X4 projMatrix = cam.ProjectionMatrix();
  XMMATRIX xmProj = XMLoadFloat4x4(&projMatrix);

  for (size_t i = 0; i < g_instanceCount; ++i) {
    const InstanceData& instance = g_instanceBuffer[i];
    XMMATRIX xmWorld = XMLoadFloat4x4(&instance.world);

    XMMATRIX xmMVP = xmWorld * xmView * xmProj;
    XMMATRIX xmInvMVP = XMMatrixInverse(nullptr, xmMVP);

    bool culled = false;
    for (auto plane : frustum.planes) {
      Plane localPlane = TransformPlane(xmInvMVP, plane);
      if (!IntersectAABBPlane(instanceAABB, localPlane)) {
        culled = true;
        break;
      }
    }

    if (!culled) {
      g_culledInstanceIndices[g_cullCounter++] = i;
    }
  }
}

void CullInstancesWorldSpace(const dxh::PerspectiveCamera& cam)
{
  g_cullCounter = 0;

  Frustum frustum = CameraFrustumNDC();
  AABB instanceAABB = {{-0.5f, -0.5f, -0.5f}, {0.5f, 0.5f, 0.5f}};

  XMFLOAT4X4 viewMatrix = cam.ViewMatrix();
  XMMATRIX xmView = XMLoadFloat4x4(&viewMatrix);

  XMFLOAT4X4 projMatrix = cam.ProjectionMatrix();
  XMMATRIX xmProj = XMLoadFloat4x4(&projMatrix);

  Frustum worldFrustum;
  {
    XMMATRIX xmInvVP = XMMatrixInverse(nullptr, xmView * xmProj);
    for (int i = 0; i < 6; ++i) {
      worldFrustum.planes[i] = TransformPlane(xmInvVP, frustum.planes[i]);
    }
  }

  for (size_t i = 0; i < g_instanceCount; ++i) {
    const InstanceData& instance = g_instanceBuffer[i];
    XMMATRIX xmWorld = XMLoadFloat4x4(&instance.world);
    AABB worldAABB = TransformAABB(xmWorld, instanceAABB);

    bool culled = false;
    for (auto worldPlane : worldFrustum.planes) {
      if (!IntersectAABBPlane(worldAABB, worldPlane)) {
        culled = true;
        break;
      }
    }

    if (!culled) {
      g_culledInstanceIndices[g_cullCounter++] = i;
    }
  }
}

void CullInstances(const dxh::PerspectiveCamera& cam, FrustumCullingSpace space)
{
  switch (space) {
    case FrustumCullingSpace::Local:
      CullInstancesLocalSpace(cam);
      break;
    case FrustumCullingSpace::World:
      CullInstancesWorldSpace(cam);
      break;
    default:
      break;
  }
}