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
  return m_commandBuffer->Begin();
}

bool RenderFrameContext::EndActiveCommandBuffer() {
  assert(IsBeginActiveCommandbuffer);
  IsBeginActiveCommandbuffer = false;
  return m_commandBuffer->End();
}

void RenderFrameContext::Destroy() {
  if (SceneRenderTargetPtr) {
    SceneRenderTargetPtr->Return();
    SceneRenderTargetPtr.reset();
  }

  if (m_commandBuffer) {
    assert(g_rhi->GetCommandBufferManager());
    g_rhi->GetCommandBufferManager()->ReturnCommandBuffer(m_commandBuffer);
    m_commandBuffer = nullptr;
  }

  FrameIndex = -1;
}

}  // namespace game_engine