
#include "gfx/rhi/vulkan/render_frame_context_vk.h"

#include "gfx/rhi/vulkan/rhi_vk.h"

namespace game_engine {

void RenderFrameContextVk::Destroy() {
  if (SceneRenderTargetPtr) {
    SceneRenderTargetPtr->Return();
    SceneRenderTargetPtr.reset();
  }

  if (CommandBuffer) {
    assert(g_rhi_vk->GetCommandBufferManager());
    g_rhi_vk->GetCommandBufferManager()->ReturnCommandBuffer(CommandBuffer);
    CommandBuffer = nullptr;
  }

  FrameIndex = -1;
}

bool RenderFrameContextVk::BeginActiveCommandBuffer() {
  assert(!IsBeginActiveCommandbuffer);
  IsBeginActiveCommandbuffer = true;
  return CommandBuffer->Begin();
}

bool RenderFrameContextVk::EndActiveCommandBuffer() {
  assert(IsBeginActiveCommandbuffer);
  IsBeginActiveCommandbuffer = false;
  return CommandBuffer->End();
}

void RenderFrameContextVk::SubmitCurrentActiveCommandBuffer(
    ECurrentRenderPass InCurrentRenderPass) {
  SwapchainImageVk* swapchainImageVk
      = (SwapchainImageVk*)g_rhi_vk->m_swapchain_.GetSwapchainImage(FrameIndex);

  switch (InCurrentRenderPass) {
    case RenderFrameContextVk::ShadowPass:
      QueueSubmitCurrentActiveCommandBuffer(
          swapchainImageVk->RenderFinishedAfterShadow);
      break;
    case RenderFrameContextVk::BasePass:
      QueueSubmitCurrentActiveCommandBuffer(
          swapchainImageVk->RenderFinishedAfterBasePass);
      break;
    default:
      break;
  }
}

void RenderFrameContextVk::QueueSubmitCurrentActiveCommandBuffer(
    SemaphoreVk* InSignalSemaphore) {
  if (CommandBuffer) {
    // TODO: temoporary removed
    // CommandBuffer->End();

    g_rhi_vk->QueueSubmit(shared_from_this(), InSignalSemaphore);
    g_rhi_vk->GetCommandBufferManager()->ReturnCommandBuffer(CommandBuffer);

    // get new command buffer
    CommandBuffer = g_rhi_vk->CommandBufferManager->GetOrCreateCommandBuffer();
    g_rhi_vk->m_swapchain_.GetSwapchainImage(FrameIndex)->CommandBufferFence
        = (VkFence)CommandBuffer->GetFenceHandle();
  }
}

}  // namespace game_engine
