#pragma once


#define DECLARE_VERTEX_PROPERTY(type, name) \
  type name;                                \
  static UINT name##_offset;

#define DEFINE_VERTEX_PROPERTY(cls, name) UINT cls::name##_offset = offsetof(cls, name);

namespace dxh
{

struct SimpleVertex {
  using RGBA8UINT = std::array<uint8_t, 4>;
  DECLARE_VERTEX_PROPERTY(DirectX::XMFLOAT3, position)
  DECLARE_VERTEX_PROPERTY(RGBA8UINT, color)
  DECLARE_VERTEX_PROPERTY(DirectX::XMFLOAT3, normal)
  static constexpr UINT inputLayoutCount = 3;
  static D3D12_INPUT_ELEMENT_DESC inputLayout[inputLayoutCount];
};

}  // namespace dxh