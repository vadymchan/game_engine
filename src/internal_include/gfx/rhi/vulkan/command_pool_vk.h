#ifndef GAME_ENGINE_COMMAND_POOL_VK_H
#define GAME_ENGINE_COMMAND_POOL_VK_H

#include "gfx/rhi/lock.h"
#include "gfx/rhi/vulkan/command_buffer_vk.h"
#include "gfx/rhi/command_buffer_manager.h"


#include <vulkan/vulkan.h>

#include <vector>

namespace game_engine {

// TODO: consider name CommandPoolVk
class CommandBufferManagerVk : public ICommandBufferManager {
  public:
  virtual ~CommandBufferManagerVk() { releaseInternal(); }

  //~CommandBufferManagerVk() {
  //  for (auto* commandBuffer : usedCommandBuffers) {
  //    delete commandBuffer;  // Ensure the command buffers are not in use
  //  }
  //  for (auto* commandBuffer : availableCommandBuffers) {
  //    delete commandBuffer;
  //  }
  //  vkDestroyCommandPool(g_rhi_vk->m_device_, commandPool, nullptr);
  //}

  virtual bool createPool(uint32_t QueueIndex);

  virtual void release() override { releaseInternal(); }

  void releaseInternal();

  const VkCommandPool& getPool() const { return m_commandPool_; }

  virtual CommandBuffer* getOrCreateCommandBuffer() override;

  void returnCommandBuffer(CommandBuffer* commandBuffer) override;

  private:
  
  VkCommandPool m_commandPool_;
  MutexLock     m_commandListLock_;

  std::vector<CommandBufferVk*> m_usingCommandBuffers_;
  std::vector<CommandBufferVk*> m_availableCommandBuffers_;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_COMMAND_POOL_VK_H