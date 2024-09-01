#ifndef GAME_ENGINE_RENDER_FRAME_CONTEXT_VK_H
#define GAME_ENGINE_RENDER_FRAME_CONTEXT_VK_H

#include "gfx/rhi/render_frame_context.h"
#include "gfx/rhi/semaphore_manager.h"
#include "gfx/rhi/vulkan/command_buffer_vk.h"
#include "gfx/rhi/vulkan/render_target_vk.h"

#include <memory>

namespace game_engine {

struct RenderFrameContextVk : public RenderFrameContext {
  public:
  // ======= BEGIN: public constructors =======================================

  RenderFrameContextVk() = default;

  RenderFrameContextVk(CommandBuffer* commandBuffer)
      : RenderFrameContext(commandBuffer) {}

  // ======= END: public constructors   =======================================

  // ======= BEGIN: public destructor =========================================

  virtual ~RenderFrameContextVk() {}

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overridden methods =================================

  virtual void submitCurrentActiveCommandBuffer(
      ECurrentRenderPass currentRenderPass) override;

  virtual void queueSubmitCurrentActiveCommandBuffer(
      ISemaphore* signalSemaphore);

  // ======= END: public overridden methods   =================================

  // ======= BEGIN: public misc fields ========================================

  ISemaphore* m_currentWaitSemaphore_ = nullptr;

  // ======= END: public misc fields   ========================================

  protected:
  // ======= BEGIN: protected misc fields =====================================

  CommandBuffer* m_commandBuffer_ = nullptr;

  // ======= END: protected misc fields   =====================================
};
}  // namespace game_engine

#endif  // GAME_ENGINE_RENDER_FRAME_CONTEXT_VK_H