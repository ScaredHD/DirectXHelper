#include "Instances.h"

size_t g_instanceCount = 1000;

float GridSize() { return 2.5f * std::sqrt(static_cast<float>(g_instanceCount)); }

float g_fieldSize = GridSize();

int GridRowCount()
{
  return static_cast<int>(std::ceilf(std::sqrt(static_cast<float>(g_instanceCount))));
}

int GridColCount()
{
  return static_cast<int>(std::sqrt(g_instanceCount));
}

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

DirectX::XMFLOAT3 InstancePosition(size_t index)
{
  auto [u, v] = GridCoordSNorm(GridCoord(index));
  auto x = u * g_fieldSize * 0.5f;
  auto z = v * g_fieldSize * 0.5f;
  return {x, 0.0f, z};
}

DirectX::XMFLOAT4X4 InstanceWorldMatrix(size_t index)
{
  using namespace DirectX;
  auto pos = InstancePosition(index);
  XMMATRIX world = XMMatrixTranslation(pos.x, pos.y, pos.z);
  XMFLOAT4X4 worldMat;
  XMStoreFloat4x4(&worldMat, world);
  return worldMat;
}

InstanceData GetInstance(size_t index)
{
  InstanceData instance;
  instance.world = InstanceWorldMatrix(index);
  float r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
  float g = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
  float b = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
  instance.color = {r, g, b, 1.0f};
  return instance;
}

std::vector<InstanceData> GetInstanceData()
{
  std::vector<InstanceData> instances;
  instances.reserve(g_instanceCount);
  for (size_t i = 0; i < g_instanceCount; ++i) {
    instances.push_back(GetInstance(i));
  }
  return instances;
}

std::vector<InstanceData> g_instanceBuffer = GetInstanceData();