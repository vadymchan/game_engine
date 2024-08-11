#include "gfx/rhi/render_frame_context.h"

#include "gfx/rhi/rhi.h"

#include <cassert>

namespace game_engine {

RenderFrameContext::~RenderFrameContext() {
  Destroy();
}

bool RenderFrameContext::BeginActiveCommandBuffer() {
  assert(!IsBeginActiveCommandbuffer);
  IsBeginActiveCommandbuffer = true;
  return m_commandBuffer_->Begin();
}

bool RenderFrameContext::EndActiveCommandBuffer() {
  assert(IsBeginActiveCommandbuffer);
  IsBeginActiveCommandbuffer = false;
  return m_commandBuffer_->End();
}

void RenderFrameContext::Destroy() {
  if (SceneRenderTargetPtr) {
    SceneRenderTargetPtr->Return();
    SceneRenderTargetPtr.reset();
  }

  if (m_commandBuffer_) {
    assert(g_rhi->GetCommandBufferManager());
    g_rhi->GetCommandBufferManager()->ReturnCommandBuffer(m_commandBuffer_);
    m_commandBuffer_ = nullptr;
  }

  FrameIndex = -1;
}

}  // namespace game_engine