#ifndef DXH_SHADER_H_
#define DXH_SHADER_H_

#include <d3d12.h>
#include <string>
#include <wrl/client.h>

namespace dxh
{

class Shader
{
public:
  explicit Shader(
    const std::wstring& path,
    const char* entryPoint,
    UINT compileFlags,
    const char* target
  );

  D3D12_SHADER_BYTECODE ByteCode() const;

  Microsoft::WRL::ComPtr<ID3DBlob> error;

private:
  Microsoft::WRL::ComPtr<ID3DBlob> blob_;
};

class VertexShader : public Shader
{
public:
  explicit VertexShader(const std::wstring& path, const char* entryPoint, UINT compileFlags)
      : Shader{path, entryPoint, compileFlags, "vs_5_0"}
  {
  }
};

class PixelShader : public Shader
{
public:
  explicit PixelShader(const std::wstring& path, const char* entryPoint, UINT compileFlags)
      : Shader{path, entryPoint, compileFlags, "ps_5_0"}
  {
  }
};

D3D12_SHADER_BYTECODE ShaderByteCode(
  const std::wstring& path,
  const char* entryPoint,
  UINT compileFlags,
  const char* target
);


}  // namespace dxh


#endif  // DXH_SHADER_H_