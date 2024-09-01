#ifndef GAME_ENGINE_COMMAND_POOL_VK_H
#define GAME_ENGINE_COMMAND_POOL_VK_H

#include "gfx/rhi/command_buffer_manager.h"
#include "gfx/rhi/lock.h"
#include "gfx/rhi/vulkan/command_buffer_vk.h"

#include <vulkan/vulkan.h>

#include <vector>

namespace game_engine {

// TODO: consider name CommandPoolVk
class CommandBufferManagerVk : public ICommandBufferManager {
  public:
  // ======= BEGIN: public destructor =========================================

  virtual ~CommandBufferManagerVk() { releaseInternal(); }

  //~CommandBufferManagerVk() {
  //  for (auto* commandBuffer : usedCommandBuffers) {
  //    delete commandBuffer;  // Ensure the command buffers are not in use
  //  }
  //  for (auto* commandBuffer : availableCommandBuffers) {
  //    delete commandBuffer;
  //  }
  //  vkDestroyCommandPool(g_rhiVk->m_device_, commandPool, nullptr);
  //}

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overridden methods =================================

  virtual bool createPool(uint32_t QueueIndex);

  virtual void release() override { releaseInternal(); }

  void returnCommandBuffer(CommandBuffer* commandBuffer) override;

  virtual CommandBuffer* getOrCreateCommandBuffer() override;

  // ======= END: public overridden methods   =================================

  // ======= BEGIN: public getters ============================================

  const VkCommandPool& getPool() const { return m_commandPool_; }

  // ======= END: public getters   ============================================

  // ======= BEGIN: public misc methods =======================================

  void releaseInternal();

  // ======= END: public misc methods   =======================================

  private:
  // ======= BEGIN: private misc fields =======================================

  VkCommandPool m_commandPool_;
  MutexLock     m_commandListLock_;

  std::vector<CommandBufferVk*> m_usingCommandBuffers_;
  std::vector<CommandBufferVk*> m_availableCommandBuffers_;

  // ======= END: private misc fields   =======================================
};

}  // namespace game_engine

#endif  // GAME_ENGINE_COMMAND_POOL_VK_H