#ifndef GAME_ENGINE_RENDER_FRAME_CONTEXT_DX12_H
#define GAME_ENGINE_RENDER_FRAME_CONTEXT_DX12_H

#include "gfx/rhi/render_frame_context.h"

namespace game_engine {

struct jRenderFrameContext_DX12 : public jRenderFrameContext {
  jRenderFrameContext_DX12() = default;

  jRenderFrameContext_DX12(jCommandBuffer* InCommandBuffer)
      : jRenderFrameContext(InCommandBuffer) {}

  virtual ~jRenderFrameContext_DX12() {}

  // TODO: seems this method is not used
  virtual void QueueSubmitCurrentActiveCommandBuffer();
  virtual void SubmitCurrentActiveCommandBuffer(
      ECurrentRenderPass InCurrentRenderPass) override;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_RENDER_FRAME_CONTEXT_DX12_H