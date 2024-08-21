#include "gfx/rhi/render_frame_context.h"

#include "gfx/rhi/rhi.h"

#include <cassert>

namespace game_engine {

RenderFrameContext::~RenderFrameContext() {
  destroy();
}

bool RenderFrameContext::beginActiveCommandBuffer() {
  assert(!m_isBeginActiveCommandbuffer_);
  m_isBeginActiveCommandbuffer_ = true;
  return m_commandBuffer_->begin();
}

bool RenderFrameContext::endActiveCommandBuffer() {
  assert(m_isBeginActiveCommandbuffer_);
  m_isBeginActiveCommandbuffer_ = false;
  return m_commandBuffer_->end();
}

void RenderFrameContext::destroy() {
  if (m_sceneRenderTargetPtr_) {
    m_sceneRenderTargetPtr_->returnRt();
    m_sceneRenderTargetPtr_.reset();
  }

  if (m_commandBuffer_) {
    assert(g_rhi->getCommandBufferManager());
    g_rhi->getCommandBufferManager()->returnCommandBuffer(m_commandBuffer_);
    m_commandBuffer_ = nullptr;
  }

  m_frameIndex_ = -1;
}

}  // namespace game_engine