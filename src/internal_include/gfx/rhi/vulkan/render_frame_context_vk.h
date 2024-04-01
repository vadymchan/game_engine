#ifndef GAME_ENGINE_RENDER_FRAME_CONTEXT_H
#define GAME_ENGINE_RENDER_FRAME_CONTEXT_H

#include "gfx/rhi/vulkan/command_buffer_vk.h"
#include "gfx/rhi/vulkan/render_target_vk.h"

#include <memory>

namespace game_engine {

struct RenderFrameContextVk
    : public std::enable_shared_from_this<RenderFrameContextVk> {
  enum ECurrentRenderPass {
    None,
    ShadowPass,
    BasePass,
  };

  RenderFrameContextVk() = default;

  RenderFrameContextVk(CommandBufferVk* InCommandBuffer)
      : CommandBuffer(InCommandBuffer) {}

  virtual ~RenderFrameContextVk() { Destroy(); }

  virtual void Destroy();

  virtual CommandBufferVk* GetActiveCommandBuffer() const {
    return CommandBuffer;
  }

  virtual bool BeginActiveCommandBuffer();

  virtual bool EndActiveCommandBuffer();

  virtual void SubmitCurrentActiveCommandBuffer(
      ECurrentRenderPass InCurrentRenderPass);

  virtual void QueueSubmitCurrentActiveCommandBuffer(
      SemaphoreVk* InSignalSemaphore);

  public:
  std::shared_ptr<SceneRenderTarget> SceneRenderTargetPtr       = nullptr;
  uint32_t                           FrameIndex                 = -1;
  bool                               UseForwardRenderer         = true;
  bool                               IsBeginActiveCommandbuffer = false;
  SemaphoreVk*                       CurrentWaitSemaphore       = nullptr;

  protected:
  CommandBufferVk* CommandBuffer = nullptr;
};
}  // namespace game_engine

#endif  // GAME_ENGINE_RENDER_FRAME_CONTEXT_H