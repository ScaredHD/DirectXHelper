#pragma once

#include "PCH.h"

namespace dxh
{


struct PerspectiveCamera {
  DirectX::XMFLOAT3 position{0.0f, 0.0f, -5.0f};
  DirectX::XMFLOAT3 lookAt{0.0f, 0.0f, 0.0f};
  DirectX::XMFLOAT3 up{0.0f, 1.0f, 0.0f};
  float fovY = DirectX::XM_PIDIV4;
  float aspectRatio = 8.f / 6.f;
  float nearZ = 0.1f;
  float farZ = 100.0f;

  DirectX::XMFLOAT4X4 ViewMatrix() const
  {
    DirectX::XMVECTOR pos = DirectX::XMLoadFloat3(&position);
    DirectX::XMVECTOR target = DirectX::XMLoadFloat3(&lookAt);
    DirectX::XMVECTOR upVec = DirectX::XMLoadFloat3(&up);
    DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(pos, target, upVec);
    DirectX::XMFLOAT4X4 viewMat;
    DirectX::XMStoreFloat4x4(&viewMat, view);
    return viewMat;
  }

  DirectX::XMFLOAT4X4 ProjectionMatrix() const
  {
    DirectX::XMMATRIX proj = DirectX::XMMatrixPerspectiveFovLH(fovY, aspectRatio, nearZ, farZ);
    DirectX::XMFLOAT4X4 projMat;
    DirectX::XMStoreFloat4x4(&projMat, proj);
    return projMat;
  }
};

}  // namespace dxh