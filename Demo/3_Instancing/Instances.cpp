#include "Instances.h"

#include <iomanip>

#include "AutoTimer.h"
#include "Culling.h"
#include "Octree.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/spdlog.h"


auto g_logger = spdlog::basic_logger_mt("basic_logger", "instances.log", true);

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

XMFLOAT3 InstanceWorldPosition(size_t index, float time = 0.f)
{
  auto pos = InstanceBasePosition(index);
  pos.y += InstanceYOffset(index, time);
  return pos;
}

XMFLOAT4X4 InstanceWorldMatrix(size_t index, float time = 0.f)
{
  auto pos = InstanceWorldPosition(index, time);
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

std::vector<InstanceData> InitInstanceData(float time = 0.f)
{
  std::vector<InstanceData> instances;
  instances.reserve(g_instanceCount);
  for (size_t i = 0; i < g_instanceCount; ++i) {
    instances.push_back(GetInstance(i, time));
  }
  return instances;
}

std::vector<InstanceData> g_instanceBuffer = InitInstanceData();
std::vector<InstanceSceneInfo> g_instanceSceneInfo(g_instanceCount);

std::unique_ptr<OctreeNode<InstanceSceneInfo>> g_sceneOctree;

void RebuildSceneOctree()
{
  // Clear existing octree node pointers

  for (size_t i = 0; i < g_instanceCount; ++i) {
    g_instanceSceneInfo[i].octreeNode = nullptr;
    g_instanceSceneInfo[i].indexInNode = 0;
  }

  float fieldSize = FieldSize();
  float halfSize = fieldSize * 0.5f + 5.f;  // add some margin
  float sceneHeight = 2.f * g_yOffsetAmplitude + 5.f;
  AABB sceneBox = {{-halfSize, -sceneHeight, -halfSize}, {halfSize, sceneHeight, halfSize}};
  g_sceneOctree = BuildSceneOctreeFromAABB(sceneBox, g_instanceSceneInfo);
}

void UpdateInstances(float time)
{
  for (size_t i = 0; i < g_instanceCount; ++i) {
    const XMFLOAT4X4& world = InstanceWorldMatrix(i, time);
    g_instanceBuffer[i].world = world;

    const XMMATRIX& xmInvWorld = XMMatrixInverse(nullptr, XMLoadFloat4x4(&world));
    XMStoreFloat4x4(&g_instanceBuffer[i].invWorld, xmInvWorld);

    g_instanceSceneInfo[i].worldPosition = InstanceWorldPosition(i, time);
    XMMATRIX xmWorld = XMLoadFloat4x4(&world);
    const auto& localAABB = AABB{{-0.5f, -0.5f, -0.5f}, {0.5f, 0.5f, 0.5f}};
    g_instanceSceneInfo[i].worldAABB = TransformAABB(xmWorld, localAABB);
    g_instanceSceneInfo[i].instanceIndex = i;
  }
}

std::vector<size_t> g_initInstanceIndices = []() {
  std::vector<size_t> indices(g_instanceCount);
  for (size_t i = 0; i < g_instanceCount; ++i) {
    indices[i] = i;
  }
  return indices;
}();

std::vector<size_t> g_culledInstanceIndices;

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

Frustum WorldSpaceFrustum(const dxh::PerspectiveCamera& cam)
{
  Frustum frustum = CameraFrustumNDC();

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
  return worldFrustum;
}

void CullInstancesLocalSpace(const dxh::PerspectiveCamera& cam)
{
  std::vector<size_t> culledIndices;

  Frustum frustum = CameraFrustumNDC();
  AABB instanceAABB = {{-0.5f, -0.5f, -0.5f}, {0.5f, 0.5f, 0.5f}};

  XMFLOAT4X4 viewMatrix = cam.ViewMatrix();
  XMMATRIX xmView = XMLoadFloat4x4(&viewMatrix);

  XMFLOAT4X4 projMatrix = cam.ProjectionMatrix();
  XMMATRIX xmProj = XMLoadFloat4x4(&projMatrix);

  for (size_t i : g_culledInstanceIndices) {
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
      culledIndices.push_back(i);
    }
  }

  g_culledInstanceIndices = culledIndices;
}

void CullInstancesWorldSpace(const dxh::PerspectiveCamera& cam)
{
  std::vector<size_t> culledIndices;
  Frustum worldFrustum = WorldSpaceFrustum(cam);

  for (size_t i : g_culledInstanceIndices) {
    const AABB& worldAABB = g_instanceSceneInfo[i].worldAABB;
    if (worldAABB.Intersect(worldFrustum)) {
      culledIndices.push_back(i);
    }
  }

  g_culledInstanceIndices = culledIndices;
}

void CullOctreeNodesImpl(
  const dxh::PerspectiveCamera& cam,
  const OctreeNode<InstanceSceneInfo>* node,
  const Frustum& worldFrustum,
  std::vector<size_t>& outCulledIndices
)
{
  if (!node->bbox.Intersect(worldFrustum)) {
    return;
  }

  if (node->IsLeaf()) {
    for (const auto* obj : node->objects) {
      outCulledIndices.push_back(obj->instanceIndex);
    }
    return;
  }

  for (const InstanceSceneInfo* obj : node->objects) {
    outCulledIndices.push_back(obj->instanceIndex);
  }

  for (const auto* child : node->children) {
    assert(child);
    CullOctreeNodesImpl(cam, child, worldFrustum, outCulledIndices);
  }
}

void CullOctreeNodes(const dxh::PerspectiveCamera& cam, const OctreeNode<InstanceSceneInfo>* node)
{
  std::vector<size_t> culledIndices;
  culledIndices.reserve(g_instanceCount);
  Frustum worldFrustum = WorldSpaceFrustum(cam);
  CullOctreeNodesImpl(cam, node, worldFrustum, culledIndices);
  g_culledInstanceIndices = culledIndices;
}

bool g_octreeBuilt = false;

#define LOG_OCTREE

void CullInstances(
  const dxh::PerspectiveCamera& cam,
  FrustumCullingSpace space,
  CullingAcceleration acceleration
)
{
  g_culledInstanceIndices = g_initInstanceIndices;

#if defined(LOG_OCTREE)
  g_logger->info("Culling instances...");
#endif

  if (acceleration == CullingAcceleration::StaticOctree ||
      acceleration == CullingAcceleration::DynamicOctree) {
    float octreeBuildTime = 0;
    float octreeCullTime = 0;
    bool isDynamic = acceleration == CullingAcceleration::DynamicOctree;
    {
      DXH_SCOPED_AUTO_TIMER_OUT_RESULT(octreeBuildTime, dxh::Microseconds);
      if (!g_octreeBuilt) {
        RebuildSceneOctree();
        g_octreeBuilt = true;
      } else {
        if (isDynamic) {
          for (size_t i = 0; i < g_instanceCount; ++i) {
            UpdateOctreeObject(*g_sceneOctree, g_instanceSceneInfo[i]);
          }
        }
      }
    }
    {
      DXH_SCOPED_AUTO_TIMER_OUT_RESULT(octreeCullTime, dxh::Microseconds);
      CullOctreeNodes(cam, g_sceneOctree.get());
    }
#if defined(LOG_OCTREE)
    g_logger->info("  Octree build/update time: {} ms", octreeBuildTime / 1000.f);
    g_logger->info("  Octree culling time: {} ms", octreeCullTime / 1000.f);
    g_logger->info("  Instances left: {}", g_culledInstanceIndices.size());
#endif
  }

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

#if defined(LOG_OCTREE)
  g_logger->info("Final instances after culling: {}", g_culledInstanceIndices.size());
  g_logger->info("Culling done.\n");
#endif
}