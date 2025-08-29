#pragma once

#include "PCH.h"

namespace dxh
{
class RootSignature
{
public:
  CD3DX12_ROOT_PARAMETER1 Parameter(int index) const { return params[index]; }
  size_t ParameterCount() const { return params.size(); }

private:
  std::vector<CD3DX12_ROOT_PARAMETER1> params;
};


};  // namespace dxh