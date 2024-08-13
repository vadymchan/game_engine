#include "gfx/rhi/dx12/shader_dx12.h"


namespace game_engine {

CompiledShaderDx12::~CompiledShaderDx12() {
  ShaderBlob.Reset();
  ShaderBlob = nullptr;
}


}