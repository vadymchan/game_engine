// shader_factory.h

#pragma once

#include "i_shader.h"
#include "dx12_shader.h"
#include "vulkan_shader.h"
#include "../renderer/render_api_type.h"
#include <memory>

namespace GameEngine {

    class ShaderFactory {
    public:
        static std::unique_ptr<IShader<DX12Shader>> CreateDX12Shader();
        static std::unique_ptr<IShader<VulkanShader>> CreateVulkanShader();
        //static std::unique_ptr<IShader> CreateShader(RenderAPI api); // currently not work due to CRTP pattern
    };

}  // namespace GameEngine
