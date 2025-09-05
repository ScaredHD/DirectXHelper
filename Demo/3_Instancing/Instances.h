#pragma once

#include "Camera.h"
#include "PCH.h"


struct InstanceData {
  DirectX::XMFLOAT4X4 world;
  DirectX::XMFLOAT4X4 invWorld;
  DirectX::XMFLOAT4 albedo;
};

extern size_t g_instanceCount;
extern std::vector<InstanceData> g_instanceBuffer;

extern std::vector<size_t> g_culledInstanceIndices;
extern size_t g_cullCounter;

void UpdateInstancePosition(float time);

void CullInstances(const dxh::PerspectiveCamera& cam);