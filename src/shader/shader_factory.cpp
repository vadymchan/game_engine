// shader_factory.cpp

#include "../include/game_engine/shader/shader_factory.h"
#include "../include/game_engine/shader/dx12_shader.h"
#include "../include/game_engine/shader/vulkan_shader.h"

namespace GameEngine {

    std::unique_ptr<IShader> ShaderFactory::CreateShader(RenderAPI api) {
        switch (api) {
        case RenderAPI::DX12:
            return std::make_unique<DX12Shader>();
        case RenderAPI::VULKAN:
            return std::make_unique<VulkanShader>();
            // add more cases as needed...
        default:
            // handle invalid API...
            return nullptr;
        }
    }

}  // namespace GameEngine
