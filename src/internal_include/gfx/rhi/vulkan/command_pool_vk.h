#ifndef GAME_ENGINE_COMMAND_POOL_VK_H
#define GAME_ENGINE_COMMAND_POOL_VK_H

#include "gfx/rhi/lock.h"
#include "gfx/rhi/vulkan/command_buffer_vk.h"

#include <vulkan/vulkan.h>

#include <vector>

namespace game_engine {

// TODO: consider name CommandPoolVk
class CommandBufferManagerVk {
  public:
  virtual ~CommandBufferManagerVk() { ReleaseInternal(); }

  //~CommandBufferManagerVk() {
  //  for (auto* commandBuffer : usedCommandBuffers) {
  //    delete commandBuffer;  // Ensure the command buffers are not in use
  //  }
  //  for (auto* commandBuffer : availableCommandBuffers) {
  //    delete commandBuffer;
  //  }
  //  vkDestroyCommandPool(g_rhi_vk->m_device_, commandPool, nullptr);
  //}

  bool CreatePool(uint32_t QueueIndex);

  void Release() { ReleaseInternal(); }

  void ReleaseInternal();

  const VkCommandPool& GetPool() const { return CommandPool; }

  CommandBufferVk* GetOrCreateCommandBuffer();

  void ReturnCommandBuffer(CommandBufferVk* commandBuffer);

  private:
  
  VkCommandPool CommandPool;
  MutexLock     CommandListLock;

  std::vector<CommandBufferVk*> UsingCommandBuffers;
  std::vector<CommandBufferVk*> AvailableCommandBuffers;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_COMMAND_POOL_VK_H