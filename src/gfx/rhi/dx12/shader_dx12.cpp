#include "gfx/rhi/dx12/shader_dx12.h"


namespace game_engine {

CompiledShaderDx12::~CompiledShaderDx12() {
  m_shaderBlob_.Reset();
  m_shaderBlob_ = nullptr;
}


}