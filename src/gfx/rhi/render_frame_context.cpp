#include "gfx/rhi/render_frame_context.h"

#include "gfx/rhi/rhi.h"

#include <cassert>

namespace game_engine {

jRenderFrameContext::~jRenderFrameContext() {
  Destroy();
}

bool jRenderFrameContext::BeginActiveCommandBuffer() {
  assert(!IsBeginActiveCommandbuffer);
  IsBeginActiveCommandbuffer = true;
  return CommandBuffer->Begin();
}

bool jRenderFrameContext::EndActiveCommandBuffer() {
  assert(IsBeginActiveCommandbuffer);
  IsBeginActiveCommandbuffer = false;
  return CommandBuffer->End();
}

void jRenderFrameContext::Destroy() {
  if (SceneRenderTargetPtr) {
    SceneRenderTargetPtr->Return();
    SceneRenderTargetPtr.reset();
  }

  if (CommandBuffer) {
    assert(g_rhi->GetCommandBufferManager());
    g_rhi->GetCommandBufferManager()->ReturnCommandBuffer(CommandBuffer);
    CommandBuffer = nullptr;
  }

  FrameIndex = -1;
}

}  // namespace game_engine