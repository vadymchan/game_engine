#pragma once

#include "i_shader.h"
#include <d3d12.h>

namespace GameEngine {

    class DX12Shader : public IShader<DX12Shader> {
    public:
        DX12Shader();
        ~DX12Shader();

        bool CompileShaderImplementation(const std::string& source, ShaderType type);
        void ReleaseShaderImplementation();

        // other shader-related methods...

    private:
        ID3DBlob* vertexShaderBlob; // Blob for holding vertex shader bytecode
        ID3DBlob* pixelShaderBlob; // Blob for holding pixel shader bytecode
        // Other data members as needed...
    };

}  // namespace GameEngine

