#pragma once
#include "../include/game_engine/shader/vulkan_shader.h"



namespace GameEngine {

    VulkanShader::VulkanShader() {
        // Constructor code...
    }

    VulkanShader::~VulkanShader() {
        Destroy();
    }

    bool VulkanShader::InitializeImplementation(const std::string& shaderFile) {
        // Initialization code for Vulkan shader...
        return true;
    }

    void VulkanShader::DestroyImplementation() {
        // Clean-up code for Vulkan shader...
    }

    void VulkanShader::BindImplementation() {
        // Bind Vulkan shader...
    }

    void VulkanShader::UnbindImplementation() {
        // Unbind Vulkan shader...
    }

    // other shader-related methods...

}  // namespace GameEngine
