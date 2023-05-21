#pragma once
#include "../include/game_engine/shader/dx12_shader.h"
#include <d3dcompiler.h>

namespace GameEngine {

    DX12Shader::DX12Shader() : vertexShaderBlob(nullptr), pixelShaderBlob(nullptr) {
        // Constructor code...
    }

    DX12Shader::~DX12Shader() {
        ReleaseShaderImplementation();
    }

    bool DX12Shader::CompileShaderImplementation(const std::string& source, ShaderType type) {
        // Shader compilation code for DX12...
        // Store the compiled shader bytecode in vertexShaderBlob or pixelShaderBlob...
    }

    void DX12Shader::ReleaseShaderImplementation() {
        // Release resources associated with the shader...
    }

    // other shader-related methods...

}  // namespace GameEngine
