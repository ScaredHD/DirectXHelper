#include "D3DUtils.h"

#include <random>

namespace dxh
{
std::random_device rd{};
std::mt19937 gen{rd()};
}  // namespace dxh

float dxh::RandomFloat(float min, float max)
{
  std::uniform_real_distribution dist{min, max};
  return dist(gen);
}