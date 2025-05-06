#include "gfx/rhi/backends/vulkan/shader_vk.h"

#include "gfx/rhi/backends/vulkan/device_vk.h"
#include "utils/logger/global_logger.h"

namespace game_engine {
namespace gfx {
namespace rhi {

ShaderVk::ShaderVk(const ShaderDesc& desc, DeviceVk* device)
    : Shader(desc)
    , m_device_(device)
    , m_shaderModule_(VK_NULL_HANDLE) {
  initialize(m_desc_.code);
}

ShaderVk::~ShaderVk() {
  release();
}

bool ShaderVk::initialize(const std::vector<uint8_t>& newShaderCode) {
  m_desc_.code = newShaderCode;

  VkShaderModuleCreateInfo createInfo = {};
  createInfo.sType                    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize                 = newShaderCode.size();
  createInfo.pCode                    = reinterpret_cast<const uint32_t*>(newShaderCode.data());

  if (vkCreateShaderModule(m_device_->getDevice(), &createInfo, nullptr, &m_shaderModule_) != VK_SUCCESS) {
    // TODO: add log message from vkCreateShaderModule (if there's a need)
    GlobalLogger::Log(LogLevel::Error, "Failed to create Vulkan shader module ");
    return false;
  }

  return true;
}

void ShaderVk::release() {
  if (!m_device_) {
    GlobalLogger::Log(LogLevel::Error, "Device is null in ShaderVk destructor");
    return;
  }

  if (m_shaderModule_ != VK_NULL_HANDLE) {
    vkDestroyShaderModule(m_device_->getDevice(), m_shaderModule_, nullptr);
    m_shaderModule_ = VK_NULL_HANDLE;
  }
}

}  // namespace rhi
}  // namespace gfx
}  // namespace game_engine