#include "gfx/rhi/dx12/shader_dx12.h"


namespace game_engine {

jCompiledShader_DX12::~jCompiledShader_DX12() {
  ShaderBlob.Reset();
  ShaderBlob = nullptr;
}


}