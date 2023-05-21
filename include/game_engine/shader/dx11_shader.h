#pragma once
#ifndef GAME_ENGINE_DX11_SHADER_H
#define GAME_ENGINE_DX11_SHADER_H

#include "i_shader.h"
#include <d3d11.h>

namespace GameEngine {

    class DX11Shader : public IShader<DX11Shader> {
    public:
        DX11Shader();
        ~DX11Shader();

        bool CompileShaderImplementation(const std::string& source, ShaderType type);
        void ReleaseShaderImplementation();

        // other shader-related methods...

    private:
        ID3DBlob* vertexShaderBlob; // Blob for holding vertex shader bytecode
        ID3DBlob* pixelShaderBlob; // Blob for holding pixel shader bytecode
        // Other data members as needed...
    };

}  // namespace GameEngine

#endif  // GAME_ENGINE_DX11_SHADER_H
