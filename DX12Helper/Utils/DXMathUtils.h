#pragma once

#include "DirectXMath.h"

DirectX::XMVECTOR ToXMVector(const DirectX::XMFLOAT3& v);
DirectX::XMVECTOR ToXMVector(const DirectX::XMFLOAT4& v);
DirectX::XMFLOAT3 ToXMFLOAT3(const DirectX::XMVECTOR& v);
DirectX::XMFLOAT4 ToXMFLOAT4(const DirectX::XMVECTOR& v);

namespace dxh
{


class XMFloat4
{
public:
  XMFloat4() : vec(DirectX::XMVectorZero()) {}
  XMFloat4(float x, float y, float z, float w) : vec(DirectX::XMVectorSet(x, y, z, w)) {}
  XMFloat4(const DirectX::XMFLOAT4& f) : vec(ToXMVector(f)) {}
  XMFloat4(const DirectX::XMVECTOR& v) : vec(v) {}

  float X() const { return DirectX::XMVectorGetX(vec); }
  float Y() const { return DirectX::XMVectorGetY(vec); }
  float Z() const { return DirectX::XMVectorGetZ(vec); }
  float W() const { return DirectX::XMVectorGetW(vec); }

private:
  DirectX::XMVECTOR vec;
};




}  // namespace dxh
