#pragma once

#include "PCH.h"

struct InstanceData {
  DirectX::XMFLOAT4X4 world;
  DirectX::XMFLOAT4X4 invWorld;
  DirectX::XMFLOAT4 albedo;
};

extern size_t g_instanceCount;
extern std::vector<InstanceData> g_instanceBuffer;

void UpdateInstancePosition(float time);