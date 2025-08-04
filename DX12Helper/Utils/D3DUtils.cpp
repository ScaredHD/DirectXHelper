#include "D3DUtils.h"

#include <random>

namespace
{
std::random_device rd{};
std::mt19937 gen{rd()};
}  // namespace

namespace dxh
{


float RandomFloat(float min, float max)
{
  std::uniform_real_distribution dist{min, max};
  return dist(gen);
}

CD3DX12_VIEWPORT MakeViewport(POINT min, POINT max)
{
  return CD3DX12_VIEWPORT{
    static_cast<FLOAT>(min.x),
    static_cast<FLOAT>(min.y),
    static_cast<FLOAT>(max.x - min.x),
    static_cast<FLOAT>(max.y - min.y),
  };
}

D3D12_RECT MakeScissorRect(POINT min, POINT max)
{
  return {min.x, min.y, max.x, max.y};
}

}  // namespace dxh