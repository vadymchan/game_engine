#include "gfx/rhi/render_frame_context.h"

#include "gfx/rhi/rhi.h"

#include <cassert>

namespace game_engine {

RenderFrameContext::~RenderFrameContext() {
  Destroy();
}

bool RenderFrameContext::BeginActiveCommandBuffer() {
  assert(!m_isBeginActiveCommandbuffer_);
  m_isBeginActiveCommandbuffer_ = true;
  return m_commandBuffer_->Begin();
}

bool RenderFrameContext::EndActiveCommandBuffer() {
  assert(m_isBeginActiveCommandbuffer_);
  m_isBeginActiveCommandbuffer_ = false;
  return m_commandBuffer_->End();
}

void RenderFrameContext::Destroy() {
  if (m_sceneRenderTargetPtr_) {
    m_sceneRenderTargetPtr_->Return();
    m_sceneRenderTargetPtr_.reset();
  }

  if (m_commandBuffer_) {
    assert(g_rhi->GetCommandBufferManager());
    g_rhi->GetCommandBufferManager()->ReturnCommandBuffer(m_commandBuffer_);
    m_commandBuffer_ = nullptr;
  }

  m_frameIndex_ = -1;
}

}  // namespace game_engine