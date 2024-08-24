#ifndef GAME_ENGINE_COMMAND_BUFFER_VK_H
#define GAME_ENGINE_COMMAND_BUFFER_VK_H

#include "gfx/rhi/vulkan/fence_vk.h"
#include "gfx/rhi/command_buffer_manager.h"

#include <vulkan/vulkan.h>

namespace game_engine {

class CommandBufferVk : public CommandBuffer {
  public:
  ~CommandBufferVk() {
    // Clean-up code if needed
  }

  virtual bool begin() const override;

  virtual bool end() const override;

  // TODO: add flags parameter
  virtual void reset(/*VkCommandBufferResetFlags flags = 0*/) const override;

  VkCommandBuffer& getRef() { return m_commandBuffer_; }

  virtual void* getNativeHandle() const override {
    return m_commandBuffer_;
  }

  // TODO: consider remove that
  virtual void* getFenceHandle() const override {
    return m_fence_ ? m_fence_->getHandle() : nullptr;
  }

  virtual IFence* getFence() const override { return m_fence_; }

  virtual void setFence(IFence* fence) { m_fence_ = fence; }

  private:
  IFence*         m_fence_         = nullptr;
  VkCommandBuffer m_commandBuffer_ = nullptr;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_COMMAND_BUFFER_VK_H