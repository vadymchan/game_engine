#include "gfx/rhi/backends/dx12/shader_dx12.h"

#include "gfx/rhi/backends/dx12/device_dx12.h"
#include "utils/logger/global_logger.h"

#ifdef GAME_ENGINE_RHI_DX12

#include <d3dcompiler.h>

namespace game_engine {
namespace gfx {
namespace rhi {

ShaderDx12::ShaderDx12(const ShaderDesc& desc, DeviceDx12* device)
    : Shader(desc)
    , m_device_(device) {
  initialize(m_desc_.code);
}

bool ShaderDx12::initialize(const std::vector<uint8_t>& newShaderCode) {
  m_desc_.code = newShaderCode;

  // Create a blob to hold the shader bytecode
  HRESULT hr = D3DCreateBlob(newShaderCode.size(), &m_shaderBlob_);
  if (FAILED(hr)) {
    GlobalLogger::Log(LogLevel::Error, "Failed to create shader blob for DirectX 12");
    return false;
  }

  // Copy the shader bytecode into the blob
  memcpy(m_shaderBlob_->GetBufferPointer(), newShaderCode.data(), newShaderCode.size());

  m_bytecode_.pShaderBytecode = m_shaderBlob_->GetBufferPointer();
  m_bytecode_.BytecodeLength  = m_shaderBlob_->GetBufferSize();

  return true;
}

void ShaderDx12::release() {
  if (m_shaderBlob_) {
    m_shaderBlob_.Reset();
  }

  m_bytecode_.pShaderBytecode = nullptr;
  m_bytecode_.BytecodeLength  = 0;
}

}  // namespace rhi
}  // namespace gfx
}  // namespace game_engine

#endif  // GAME_ENGINE_RHI_DX12