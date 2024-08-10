
#include "gfx/rhi/vulkan/shader_vk.h"

#include "gfx/rhi/vulkan/rhi_vk.h"

namespace game_engine {

CompiledShaderVk::~CompiledShaderVk() {
  if (ShaderStage.module) {
    vkDestroyShaderModule(g_rhi_vk->m_device_, ShaderStage.module, nullptr);
  }
  ShaderStage = {};
}
}  // namespace game_engine
