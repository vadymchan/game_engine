#ifndef GAME_ENGINE_RENDER_FRAME_CONTEXT_DX12_H
#define GAME_ENGINE_RENDER_FRAME_CONTEXT_DX12_H

#include "platform/windows/windows_platform_setup.h"

#ifdef GAME_ENGINE_RHI_DX12

#include "gfx/rhi/render_frame_context.h"

namespace game_engine {

struct RenderFrameContextDx12 : public RenderFrameContext {
  // ======= BEGIN: public constructors =======================================

  RenderFrameContextDx12() = default;

  RenderFrameContextDx12(std::shared_ptr<CommandBuffer> commandBuffer)
      : RenderFrameContext(commandBuffer) {}

  // ======= END: public constructors   =======================================

  // ======= BEGIN: public destructor =========================================

  virtual ~RenderFrameContextDx12() {}

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overridden methods =================================

  // TODO: seems this method is not used
  virtual void queueSubmitCurrentActiveCommandBuffer();
  virtual void submitCurrentActiveCommandBuffer(
      ECurrentRenderPass currentRenderPass) override;

  // ======= END: public overridden methods   =================================
};

}  // namespace game_engine

#endif  // GAME_ENGINE_RHI_DX12

#endif  // GAME_ENGINE_RENDER_FRAME_CONTEXT_DX12_H