#ifndef GAME_ENGINE_RENDER_FRAME_CONTEXT_H
#define GAME_ENGINE_RENDER_FRAME_CONTEXT_H

#include "gfx/rhi/command_buffer_manager.h"
#include "gfx/rhi/render_target.h"

#include <memory>

namespace game_engine {

struct jRenderFrameContext
    : public std::enable_shared_from_this<jRenderFrameContext> {
  enum ECurrentRenderPass {
    None,
    ShadowPass,
    BasePass,
  };

  jRenderFrameContext() = default;

  jRenderFrameContext(jCommandBuffer* InCommandBuffer)
      : CommandBuffer(InCommandBuffer) {}

  virtual ~jRenderFrameContext();

  virtual void Destroy();

  virtual jCommandBuffer* GetActiveCommandBuffer() const {
    return CommandBuffer;
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
  jCommandBuffer* CommandBuffer = nullptr;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_RENDER_FRAME_CONTEXT_H