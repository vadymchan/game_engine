#ifndef GAME_ENGINE_COMMAND_POOL_VK_H
#define GAME_ENGINE_COMMAND_POOL_VK_H

#include "gfx/rhi/lock.h"
#include "gfx/rhi/vulkan/command_buffer_vk.h"
#include "gfx/rhi/command_buffer_manager.h"


#include <vulkan/vulkan.h>

#include <vector>

namespace game_engine {

// TODO: consider name CommandPoolVk
class CommandBufferManagerVk : public CommandBufferManager {
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

  virtual bool CreatePool(uint32_t QueueIndex);

  virtual void Release() override { ReleaseInternal(); }

  void ReleaseInternal();

  const VkCommandPool& GetPool() const { return CommandPool; }

  virtual CommandBuffer* GetOrCreateCommandBuffer() override;

  void ReturnCommandBuffer(CommandBuffer* commandBuffer) override;

  private:
  
  VkCommandPool CommandPool;
  MutexLock     CommandListLock;

  std::vector<CommandBufferVk*> UsingCommandBuffers;
  std::vector<CommandBufferVk*> AvailableCommandBuffers;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_COMMAND_POOL_VK_H