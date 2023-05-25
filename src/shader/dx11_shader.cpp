#pragma once
#include "../include/game_engine/shader/dx11_shader.h"
#include <d3dcompiler.h>

namespace GameEngine {

    DX11Shader::DX11Shader() : vertexShaderBlob(nullptr), pixelShaderBlob(nullptr) {
        // Constructor code...
    }

    DX11Shader::~DX11Shader() {
        ReleaseShaderImplementation();
    }

    bool DX11Shader::CompileShaderImplementation(const std::string& source, ShaderType type) {
        // Shader compilation code for DX11...
        // Store the compiled shader bytecode in vertexShaderBlob or pixelShaderBlob...
        return true;
    }

    void DX11Shader::ReleaseShaderImplementation() {
        // Release resources associated with the shader...
    }

    // other shader-related methods...

}  // namespace GameEngine
