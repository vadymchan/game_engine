
#include "gfx/rhi/vulkan/shader_vk.h"

#include "gfx/rhi/vulkan/rhi_vk.h"

namespace game_engine {

CompiledShaderVk::~CompiledShaderVk() {
  if (m_shaderStage_.module) {
    vkDestroyShaderModule(g_rhiVk->m_device_, m_shaderStage_.module, nullptr);
  }
  m_shaderStage_ = {};
}
}  // namespace game_engine
