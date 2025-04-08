#pragma once
#include <cstdint>
#include <DirectXMath.h>
#include <dxgi.h>
#include <stdexcept>
#include <vector>

namespace dxh
{

struct DefaultVertex {
  DirectX::XMFLOAT3 pos = {0.f, 0.f, 0.f};
  DirectX::XMFLOAT3 normal = {0.f, 0.f, 0.f};
  DirectX::XMFLOAT4 color = {0.f, 0.f, 0.f, 1.f};
};

struct DefaultIndex {
  static constexpr DXGI_FORMAT format = DXGI_FORMAT_R32_UINT;

  DefaultIndex() = default;
  DefaultIndex(uint32_t idx) : idx{idx} {}
  uint32_t idx = 0;
  operator uint32_t() const { return idx; }
};

template<typename VertexType, typename IndexType>
class IFaceVertexTriangleMesh
{
public:
  struct FaceIndices {
    IndexType v0;
    IndexType v1;
    IndexType v2;
  };

  virtual FaceIndices GetFace(size_t idx) const = 0;
  virtual size_t FaceCount() const = 0;

  virtual VertexType GetVertex(size_t idx) const = 0;
  virtual size_t VertexCount() const = 0;
};

template<typename VertexType, typename IndexType>
size_t VertexArrayByteSize(const IFaceVertexTriangleMesh<VertexType, IndexType>& mesh)
{
  return mesh.VertexCount() * sizeof(VertexType);
}

template<typename VertexType, typename IndexType>
std::vector<VertexType> AssembleVertexArray(
  const IFaceVertexTriangleMesh<VertexType, IndexType>& mesh
)
{
  auto vertexCount = mesh.VertexCount();
  std::vector<VertexType> arr(vertexCount);
  for (int i = 0; i < vertexCount; ++i) {
    arr[i] = mesh.GetVertex(i);
  }
  return arr;
}


template<typename VertexType, typename IndexType>
size_t IndexArrayByteSize(const IFaceVertexTriangleMesh<VertexType, IndexType>& mesh)
{
  return mesh.FaceCount() * 3 * sizeof(IndexType);
}

template<typename VertexType, typename IndexType>
std::vector<IndexType> AssembleIndexArray(const IFaceVertexTriangleMesh<VertexType, IndexType>& mesh
)
{
  auto faceCount = mesh.FaceCount();
  std::vector<IndexType> arr(3 * faceCount);
  for (int i = 0; i < faceCount; ++i) {
    auto f = mesh.GetFace(i);
    arr[3 * i + 0] = f.v0;
    arr[3 * i + 1] = f.v1;
    arr[3 * i + 2] = f.v2;
  }
  return arr;
}


}  // namespace dxh
