

#include "gfx/rhi/vulkan/command_buffer_vk.h"
#include "utils/logger/global_logger.h"

#include <vulkan/vulkan.h>

namespace game_engine {

bool CommandBufferVk::begin() const {
  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = 0;  // TODO: consider making this configurable
  beginInfo.pInheritanceInfo
      = nullptr;        // Only necessary for secondary command buffers

  if (vkBeginCommandBuffer(m_commandBuffer_, &beginInfo) != VK_SUCCESS) {
    GlobalLogger::Log(LogLevel::Error, "Failed to begin command buffer");
    return false;
  }

  return true;
}

bool CommandBufferVk::end() const {
  if (vkEndCommandBuffer(m_commandBuffer_) != VK_SUCCESS) {
    GlobalLogger::Log(LogLevel::Error, "Failed to end command buffer");
    return false;
  }

  return true;
}

void CommandBufferVk::reset(/*VkCommandBufferResetFlags flags*/) const{
  vkResetCommandBuffer(m_commandBuffer_, /*flags*/ 0);
}

}  // namespace game_engine
