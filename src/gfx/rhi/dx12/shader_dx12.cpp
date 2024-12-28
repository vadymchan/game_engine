#include "gfx/rhi/dx12/shader_dx12.h"

#ifdef GAME_ENGINE_RHI_DX12

namespace game_engine {

CompiledShaderDx12::~CompiledShaderDx12() {
  m_shaderBlob_.reset();
  m_shaderBlob_ = nullptr;
}

}  // namespace game_engine

#endif  // GAME_ENGINE_RHI_DX12
