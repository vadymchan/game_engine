#ifndef GAME_ENGINE_RENDER_FRAME_CONTEXT_VK_H
#define GAME_ENGINE_RENDER_FRAME_CONTEXT_VK_H

#include "gfx/rhi/render_frame_context.h"
#include "gfx/rhi/semaphore_manager.h"
#include "gfx/rhi/vulkan/command_buffer_vk.h"
#include "gfx/rhi/vulkan/render_target_vk.h"

#include <memory>

namespace game_engine {

struct RenderFrameContextVk : public RenderFrameContext {
  RenderFrameContextVk() = default;

  RenderFrameContextVk(CommandBuffer* commandBuffer)
      : RenderFrameContext(commandBuffer) {}

  virtual ~RenderFrameContextVk() {}

  virtual void SubmitCurrentActiveCommandBuffer(
      ECurrentRenderPass currentRenderPass) override;

  virtual void QueueSubmitCurrentActiveCommandBuffer(
      Semaphore* InSignalSemaphore);

  public:
  Semaphore* m_currentWaitSemaphore_ = nullptr;

  protected:
  CommandBuffer* m_commandBuffer_ = nullptr;
};
}  // namespace game_engine

#endif  // GAME_ENGINE_RENDER_FRAME_CONTEXT_VK_H