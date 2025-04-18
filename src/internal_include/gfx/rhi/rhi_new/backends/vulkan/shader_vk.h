#ifndef GAME_ENGINE_SHADER_VK_H
#define GAME_ENGINE_SHADER_VK_H

#include "gfx/rhi/rhi_new/interface/shader.h"

#include <vulkan/vulkan.h>

namespace game_engine {
namespace gfx {
namespace rhi {

class DeviceVk;

/**
 * Wraps a VkShaderModule and maintains the associated shader stage and entry point.
 */
class ShaderVk : public Shader {
  public:
  /**
   * Create a Vulkan shader from shader bytecode
   */
  ShaderVk(const ShaderDesc& desc, DeviceVk* device);
  ~ShaderVk() override;

  ShaderVk(const ShaderVk&)            = delete;
  ShaderVk& operator=(const ShaderVk&) = delete;

  // Vulkan-specific methods
  VkShaderModule getShaderModule() const { return m_shaderModule_; }

  private:
  DeviceVk*      m_device_;
  VkShaderModule m_shaderModule_;
};

}  // namespace rhi
}  // namespace gfx
}  // namespace game_engine

#endif  // GAME_ENGINE_SHADER_VK_H