#pragma once

#include <string>

#include "fmt/format.h"
#include "Geometry.h"
#include "tiny_obj_loader.h"

namespace dxh
{

class ObjTriangleMesh : public IFaceVertexTriangleMesh<DefaultVertex, uint32_t>
{
public:
  explicit ObjTriangleMesh(const std::string& file)
  {
    if (!reader_.ParseFromFile(file, config_)) {
      if (!reader_.Error().empty()) {
        throw std::runtime_error{fmt::format("TinyObjReader: {}", reader_.Error())};
      }
    }
  }

  FaceIndices GetFace(size_t idx) const override
  {
    auto& shapes = reader_.GetShapes();
    for (const auto& shape : shapes) {
      auto faceCount = shape.mesh.num_face_vertices.size();
      if (idx >= faceCount) {
        idx -= faceCount;
        continue;
      }
      const auto& indices = shape.mesh.indices;
      auto i0 = static_cast<uint32_t>(indices[idx].vertex_index);
      auto i1 = static_cast<uint32_t>(indices[idx + 1].vertex_index);
      auto i2 = static_cast<uint32_t>(indices[idx + 2].vertex_index);
      return {i0, i1, i2};
    }
  }

  size_t FaceCount() const override
  {
    auto& shapes = reader_.GetShapes();

    size_t faceCount = 0;
    for (auto& shape : shapes) {
      faceCount += shape.mesh.num_face_vertices.size();
    }
    return faceCount;
  }

  DefaultVertex GetVertex(size_t idx) const override
  {
    DefaultVertex v;
    auto& attr = reader_.GetAttrib();

    const auto& vertices = attr.vertices;
    v.pos.x = vertices[3 * idx];
    v.pos.y = vertices[3 * idx + 1];
    v.pos.z = vertices[3 * idx + 2];

    const auto& normals = attr.normals;
    v.normal.x = normals[3 * idx];
    v.normal.y = normals[3 * idx + 1];
    v.normal.z = normals[3 * idx + 2];

    const auto& colors = attr.colors;
    v.color.x = colors[3 * idx];
    v.color.y = colors[3 * idx + 1];
    v.color.z = colors[3 * idx + 2];

    return v;
  }

  size_t VertexCount() const override
  {
    auto& attr = reader_.GetAttrib();
    return attr.vertices.size() / 3;
  }


private:
  tinyobj::ObjReader reader_;
  tinyobj::ObjReaderConfig config_;
};


}  // namespace dxh
