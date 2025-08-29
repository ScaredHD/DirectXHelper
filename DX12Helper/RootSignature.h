#pragma once

#include "PCH.h"

namespace dxh
{

class RootSignature
{
public:
  explicit RootSignature(
    ID3D12Device* device,
    UINT parameterCount,
    D3D12_ROOT_PARAMETER parameters[]
  )
  {
    rootParameters.resize(parameterCount);
    std::copy(parameters, parameters + parameterCount, std::begin(rootParameters));

    CD3DX12_ROOT_SIGNATURE_DESC desc{};
    desc.Init(parameterCount, parameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    DX::ThrowIfFailed(D3D12SerializeRootSignature(
      &desc, D3D_ROOT_SIGNATURE_VERSION_1_0, signatureBlob.GetAddressOf(), errorBlob.GetAddressOf()
    ));
    device->CreateRootSignature(
      0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(),
      IID_PPV_ARGS(signature.GetAddressOf())
    );
  }

  D3D12_ROOT_PARAMETER Parameter(size_t index) const { return rootParameters[index]; }
  size_t ParameterCount() const { return rootParameters.size(); }

  ID3D12RootSignature* GetRootSignature() const { return signature.Get(); }

private:
  std::vector<D3D12_ROOT_PARAMETER> rootParameters;
  Microsoft::WRL::ComPtr<ID3D12RootSignature> signature;
  Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob;
  Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
};


};  // namespace dxh