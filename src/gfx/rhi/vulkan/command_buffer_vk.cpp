

#include "gfx/rhi/vulkan/command_buffer_vk.h"
#include "utils/logger/global_logger.h"

#include <vulkan/vulkan.h>

namespace game_engine {

bool CommandBufferVk::Begin() {
  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = 0;  // TODO: consider making this configurable
  beginInfo.pInheritanceInfo
      = nullptr;        // Only necessary for secondary command buffers

  if (vkBeginCommandBuffer(CommandBuffer, &beginInfo) != VK_SUCCESS) {
    GlobalLogger::Log(LogLevel::Error, "Failed to begin command buffer");
    return false;
  }

  return true;
}

bool CommandBufferVk::End() {
  if (vkEndCommandBuffer(CommandBuffer) != VK_SUCCESS) {
    GlobalLogger::Log(LogLevel::Error, "Failed to end command buffer");
    return false;
  }

  return true;
}

void CommandBufferVk::Reset(VkCommandBufferResetFlags flags) {
  vkResetCommandBuffer(CommandBuffer, flags);
}

}  // namespace game_engine
