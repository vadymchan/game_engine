#pragma once



#include "i_shader.h"

namespace GameEngine {

    class VulkanShader : public IShader<VulkanShader> {
    public:
        VulkanShader();
        ~VulkanShader();

        bool InitializeImplementation(const std::string& shaderFile);
        void DestroyImplementation();

        void BindImplementation();
        void UnbindImplementation();

        // other shader-related methods...
    };

}  // namespace GameEngine

