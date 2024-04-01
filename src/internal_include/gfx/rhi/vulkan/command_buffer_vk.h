#ifndef GAME_ENGINE_COMMAND_BUFFER_VK_H
#define GAME_ENGINE_COMMAND_BUFFER_VK_H

#include "gfx/rhi/vulkan/fence_vk.h"

#include <vulkan/vulkan.h>

namespace game_engine {

class CommandBufferVk {
  public:
  ~CommandBufferVk() {
    // Clean-up code if needed
  }

  bool Begin();

  bool End();

  void Reset(VkCommandBufferResetFlags flags = 0);

  VkCommandBuffer& GetRef() { return CommandBuffer; }

  virtual VkCommandBuffer GetNativeHandle() const { return CommandBuffer; }

  // TODO: consider remove that
  virtual void* GetFenceHandle() const {
    return Fence ? Fence->GetHandle() : nullptr;
  }

  virtual FenceVk* GetFence() const { return Fence; }

  virtual void SetFence(FenceVk* fence) { Fence = fence; }

  private:
  FenceVk*        Fence         = nullptr;
  VkCommandBuffer CommandBuffer = nullptr;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_COMMAND_BUFFER_VK_H