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

  RenderFrameContext(CommandBuffer* commandBuffer)
      : m_commandBuffer_(commandBuffer) {}

  virtual ~RenderFrameContext();

  virtual void Destroy();

  virtual CommandBuffer* GetActiveCommandBuffer() const {
    return m_commandBuffer_;
  }

  virtual bool BeginActiveCommandBuffer();
  virtual bool EndActiveCommandBuffer();

  virtual void SubmitCurrentActiveCommandBuffer(
      ECurrentRenderPass currentRenderPass) {}

  public:
  // RaytracingScene*                   raytracingScene            = nullptr;
  std::shared_ptr<SceneRenderTarget> m_sceneRenderTargetPtr_       = nullptr;
  uint32_t                           m_frameIndex_                 = -1;
  bool                               m_useForwardRenderer_         = true;
  bool                               m_isBeginActiveCommandbuffer_ = false;

  protected:
  CommandBuffer* m_commandBuffer_ = nullptr;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_RENDER_FRAME_CONTEXT_H