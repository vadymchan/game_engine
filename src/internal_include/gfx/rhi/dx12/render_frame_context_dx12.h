#ifndef GAME_ENGINE_RENDER_FRAME_CONTEXT_DX12_H
#define GAME_ENGINE_RENDER_FRAME_CONTEXT_DX12_H

#include "gfx/rhi/render_frame_context.h"

namespace game_engine {

struct RenderFrameContextDx12 : public RenderFrameContext {
  RenderFrameContextDx12() = default;

  RenderFrameContextDx12(CommandBuffer* InCommandBuffer)
      : RenderFrameContext(InCommandBuffer) {}

  virtual ~RenderFrameContextDx12() {}

  // TODO: seems this method is not used
  virtual void QueueSubmitCurrentActiveCommandBuffer();
  virtual void SubmitCurrentActiveCommandBuffer(
      ECurrentRenderPass InCurrentRenderPass) override;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_RENDER_FRAME_CONTEXT_DX12_H