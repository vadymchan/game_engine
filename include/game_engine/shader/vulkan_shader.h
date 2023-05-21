#pragma once


#include "i_shader.h"

namespace GameEngine {

    class VulkanShader : public IShader {
    public:
        VulkanShader();
        virtual ~VulkanShader();

        bool Initialize(const std::string& shaderFile) override;
        void Destroy() override;

        void Bind() override;
        void Unbind() override;

        // other shader-related methods...
    };

}  // namespace GameEngine

