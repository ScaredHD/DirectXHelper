#pragma once

#include "PCH.h"

struct InstanceData {
  DirectX::XMFLOAT4X4 world;
  DirectX::XMFLOAT4 color;
};

extern size_t g_instanceCount;
extern std::vector<InstanceData> g_instanceBuffer;

// std::vector<InstanceData> GetInstanceData();