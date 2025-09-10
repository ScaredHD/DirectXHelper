#include "DXMathUtils.h"

DirectX::XMVECTOR ToXMVector(const DirectX::XMFLOAT3& v)
{
  return DirectX::XMLoadFloat3(&v);
}

DirectX::XMVECTOR ToXMVector(const DirectX::XMFLOAT4& v)
{
  return DirectX::XMLoadFloat4(&v);
}

DirectX::XMFLOAT3 ToXMFLOAT3(const DirectX::XMVECTOR& v)
{
  DirectX::XMFLOAT3 f;
  DirectX::XMStoreFloat3(&f, v);
  return f;
}

DirectX::XMFLOAT4 ToXMFLOAT4(const DirectX::XMVECTOR& v)
{
  DirectX::XMFLOAT4 f;
  DirectX::XMStoreFloat4(&f, v);
  return f;
}