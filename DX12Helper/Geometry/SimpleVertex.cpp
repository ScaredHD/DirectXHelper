
#include "SimpleVertex.h"

namespace dxh
{


DEFINE_VERTEX_PROPERTY(SimpleVertex, position)
DEFINE_VERTEX_PROPERTY(SimpleVertex, color)

D3D12_INPUT_ELEMENT_DESC SimpleVertex::inputLayout[SimpleVertex::inputLayoutCount] = {
  {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, SimpleVertex::position_offset,
   D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
  {"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, SimpleVertex::color_offset,
   D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
};


}  // namespace dxh