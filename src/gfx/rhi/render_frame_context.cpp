#include "gfx/rhi/render_frame_context.h"

#include "gfx/rhi/rhi.h"

#include <cassert>

namespace game_engine {

RenderFrameContext::~RenderFrameContext() {
  destroy();
}

void RenderFrameContext::destroy() {
  if (m_sceneRenderTarget_) {
    m_sceneRenderTarget_->returnRt();
    m_sceneRenderTarget_.reset();
  }

  if (m_commandBuffer_) {
    assert(g_rhi->getCommandBufferManager());
    g_rhi->getCommandBufferManager()->returnCommandBuffer(m_commandBuffer_);
    m_commandBuffer_ = nullptr;
  }

  m_frameIndex_ = -1;
}

}  // namespace game_engine
