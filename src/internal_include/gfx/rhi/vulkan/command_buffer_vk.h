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

  virtual bool Begin() const override;

  virtual bool End() const override;

  // TODO: add flags parameter
  virtual void Reset(/*VkCommandBufferResetFlags flags = 0*/) const override;

  VkCommandBuffer& GetRef() { return m_commandBuffer_; }

  virtual void* GetNativeHandle() const override {
    return m_commandBuffer_;
  }

  // TODO: consider remove that
  virtual void* GetFenceHandle() const override {
    return Fence ? Fence->GetHandle() : nullptr;
  }

  virtual Fence* GetFence() const override { return Fence; }

  virtual void SetFence(Fence* fence) { Fence = fence; }

  private:
  Fence*         Fence         = nullptr;
  VkCommandBuffer m_commandBuffer_ = nullptr;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_COMMAND_BUFFER_VK_H