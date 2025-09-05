#include "Instances.h"

using namespace DirectX;


size_t g_instanceCount = 1000;

float GridSize()
{
  return 2.5f * std::sqrt(static_cast<float>(g_instanceCount));
}

float g_fieldSize = GridSize();
float g_waveFreq = 2.f;
float g_rotSpeed = 100.f;
float g_yOffsetAmplitude = 3.f;

int GridRowCount()
{
  return static_cast<int>(std::ceilf(std::sqrt(static_cast<float>(g_instanceCount))));
}

int GridColCount()
{
  return static_cast<int>(std::sqrt(g_instanceCount));
}

using CoordI = std::pair<int, int>;
using CoordF = std::pair<float, float>;

std::pair<int, int> GridCoord(size_t index)
{
  int gridSize = static_cast<int>(std::sqrt(g_instanceCount));
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
  auto x = u * g_fieldSize * 0.5f;
  auto z = v * g_fieldSize * 0.5f;
  return {x, 0.0f, z};
}

float InstanceYOffset(size_t index, float time)
{
  auto [u, v] = GridCoordSNorm(GridCoord(index));
  return g_yOffsetAmplitude * std::sinf(g_waveFreq * (u + time)) * std::cosf(g_waveFreq * (v + time));
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