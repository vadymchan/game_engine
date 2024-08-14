#ifndef GAME_ENGINE_RENDER_FRAME_CONTEXT_H
#define GAME_ENGINE_RENDER_FRAME_CONTEXT_H

#include "gfx/rhi/command_buffer_manager.h"
#include "gfx/rhi/render_target.h"

#include <memory>

namespace game_engine {

struct RenderFrameContext
    : public std::enable_shared_from_this<RenderFrameContext> {
  enum ECurrentRenderPass {
    None,
    ShadowPass,
    BasePass,
  };

  RenderFrameContext() = default;

  RenderFrameContext(CommandBuffer* InCommandBuffer)
      : m_commandBuffer(InCommandBuffer) {}

  virtual ~RenderFrameContext();

  virtual void Destroy();

  virtual CommandBuffer* GetActiveCommandBuffer() const {
    return m_commandBuffer;
  }

  virtual bool BeginActiveCommandBuffer();
  virtual bool EndActiveCommandBuffer();

  virtual void SubmitCurrentActiveCommandBuffer(
      ECurrentRenderPass InCurrentRenderPass) {}

  public:
  // RaytracingScene*                   raytracingScene            = nullptr;
  std::shared_ptr<SceneRenderTarget> SceneRenderTargetPtr       = nullptr;
  uint32_t                           FrameIndex                 = -1;
  bool                               UseForwardRenderer         = true;
  bool                               IsBeginActiveCommandbuffer = false;

  protected:
  CommandBuffer* m_commandBuffer = nullptr;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_RENDER_FRAME_CONTEXT_H