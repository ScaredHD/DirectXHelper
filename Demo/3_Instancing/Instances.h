#pragma once

#include "Camera.h"
#include "Culling.h"
#include "PCH.h"

template<typename ObjectType>
struct OctreeNode;

struct InstanceData {
  DirectX::XMFLOAT4X4 world;
  DirectX::XMFLOAT4X4 invWorld;
  DirectX::XMFLOAT4 albedo;
};

struct InstanceSceneInfo {
  DirectX::XMFLOAT3 worldPosition;
  AABB worldAABB;
  size_t instanceIndex = -1;
  OctreeNode<InstanceSceneInfo>* octreeNode = nullptr;
  size_t indexInNode = 0;
};

extern size_t g_instanceCount;
extern std::vector<InstanceData> g_instanceBuffer;

extern std::vector<size_t> g_culledInstanceIndices;
extern size_t g_cullCounter;

void UpdateInstances(float time);

enum class FrustumCullingSpace : uint8_t { None, Local, World };
enum class CullingAcceleration : uint8_t { None, StaticOctree, DynamicOctree };

extern bool g_octreeBuilt;

void CullInstances(
  const dxh::PerspectiveCamera& cam,
  FrustumCullingSpace space = FrustumCullingSpace::World,
  CullingAcceleration acceleration = CullingAcceleration::None
);