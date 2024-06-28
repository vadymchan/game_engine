#ifndef GAME_ENGINE_RENDER_FRAME_CONTEXT_VK_H
#define GAME_ENGINE_RENDER_FRAME_CONTEXT_VK_H

#include "gfx/rhi/render_frame_context.h"
#include "gfx/rhi/semaphore_manager.h"
#include "gfx/rhi/vulkan/command_buffer_vk.h"
#include "gfx/rhi/vulkan/render_target_vk.h"

#include <memory>

namespace game_engine {

struct RenderFrameContextVk : public jRenderFrameContext {
  RenderFrameContextVk() = default;

  RenderFrameContextVk(jCommandBuffer* InCommandBuffer)
      : jRenderFrameContext(InCommandBuffer) {}

  virtual ~RenderFrameContextVk() {}

  virtual void SubmitCurrentActiveCommandBuffer(
      ECurrentRenderPass InCurrentRenderPass) override;

  virtual void QueueSubmitCurrentActiveCommandBuffer(
      jSemaphore* InSignalSemaphore);

  public:
  jSemaphore* CurrentWaitSemaphore = nullptr;

  protected:
  jCommandBuffer* CommandBuffer = nullptr;
};
}  // namespace game_engine

#endif  // GAME_ENGINE_RENDER_FRAME_CONTEXT_VK_H