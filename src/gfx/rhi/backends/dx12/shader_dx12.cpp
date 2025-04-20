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
  // Create a blob to hold the shader bytecode
  HRESULT hr = D3DCreateBlob(desc.code.size(), &m_shaderBlob_);
  if (FAILED(hr)) {
    GlobalLogger::Log(LogLevel::Error, "Failed to create shader blob for DirectX 12 shader");
    return;
  }

  // Copy the shader bytecode into the blob
  memcpy(m_shaderBlob_->GetBufferPointer(), desc.code.data(), desc.code.size());

  m_bytecode_.pShaderBytecode = m_shaderBlob_->GetBufferPointer();
  m_bytecode_.BytecodeLength  = m_shaderBlob_->GetBufferSize();

}

}  // namespace rhi
}  // namespace gfx
}  // namespace game_engine

#endif  // GAME_ENGINE_RHI_DX12