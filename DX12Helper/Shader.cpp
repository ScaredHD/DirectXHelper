#include "Shader.h"

#include "PCH.h"

namespace
{
std::string ToAsciiString(const std::wstring& str)
{
  std::string res;
  for (wchar_t ch : str) {
    res.append(1, static_cast<char>(ch));
  }
  return res;
}
}  // namespace


namespace dxh
{


Shader::Shader(
  const std::wstring& path,
  const char* entryPoint,
  UINT compileFlags,
  const char* target
)
{
  D3DCompileFromFile(
    path.c_str(), nullptr, nullptr, entryPoint, target, compileFlags, 0, blob_.GetAddressOf(),
    error.GetAddressOf()
  );
  if (error) {
    ::OutputDebugStringA(static_cast<LPCSTR>(error->GetBufferPointer()));

    std::string errorMsg = "shader compilation error: ";
    errorMsg += ToAsciiString(
      std::wstring(static_cast<const wchar_t*>(error->GetBufferPointer()), error->GetBufferSize())
    );
    throw std::runtime_error{errorMsg};
  }
}

D3D12_SHADER_BYTECODE Shader::ByteCode() const
{
  return CD3DX12_SHADER_BYTECODE{blob_.Get()};
}


}  // namespace dxh