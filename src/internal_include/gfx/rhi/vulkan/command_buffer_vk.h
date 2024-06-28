#ifndef GAME_ENGINE_COMMAND_BUFFER_VK_H
#define GAME_ENGINE_COMMAND_BUFFER_VK_H

#include "gfx/rhi/vulkan/fence_vk.h"
#include "gfx/rhi/command_buffer_manager.h"

#include <vulkan/vulkan.h>

namespace game_engine {

class CommandBufferVk : public jCommandBuffer {
  public:
  ~CommandBufferVk() {
    // Clean-up code if needed
  }

  virtual bool Begin() const override;

  virtual bool End() const override;

  // TODO: add flags parameter
  virtual void Reset(/*VkCommandBufferResetFlags flags = 0*/) const override;

  VkCommandBuffer& GetRef() { return CommandBuffer; }

  virtual void* GetNativeHandle() const override {
    return CommandBuffer;
  }

  // TODO: consider remove that
  virtual void* GetFenceHandle() const override {
    return Fence ? Fence->GetHandle() : nullptr;
  }

  virtual jFence* GetFence() const override { return Fence; }

  virtual void SetFence(jFence* fence) { Fence = fence; }

  private:
  jFence*         Fence         = nullptr;
  VkCommandBuffer CommandBuffer = nullptr;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_COMMAND_BUFFER_VK_H