#ifndef GAME_ENGINE_SHADER_DX12_H
#define GAME_ENGINE_SHADER_DX12_H

#include "gfx/rhi/shader.h"
#include "platform/windows/windows_platform_setup.h"

// TODO: remove this
#include <windows.h>

namespace game_engine {

struct CompiledShaderDx12 : public CompiledShader {
  virtual ~CompiledShaderDx12();

  ComPtr<IDxcBlob> ShaderBlob;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_SHADER_DX12_H