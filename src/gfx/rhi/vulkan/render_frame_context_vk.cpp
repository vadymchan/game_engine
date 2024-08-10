
#include "gfx/rhi/vulkan/render_frame_context_vk.h"

#include "gfx/rhi/vulkan/rhi_vk.h"

namespace game_engine {



void RenderFrameContextVk::SubmitCurrentActiveCommandBuffer(
    ECurrentRenderPass InCurrentRenderPass) {
  SwapchainImageVk* swapchainImageVk
      = (SwapchainImageVk*)g_rhi_vk->m_swapchain_->GetSwapchainImage(FrameIndex);

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
    jSemaphore* InSignalSemaphore) {
  if (CommandBuffer) {
    // TODO: temoporary removed
    // CommandBuffer->End();

    g_rhi_vk->QueueSubmit(shared_from_this(), InSignalSemaphore);
    g_rhi_vk->GetCommandBufferManager()->ReturnCommandBuffer(CommandBuffer);

    // get new command buffer
    CommandBuffer = g_rhi_vk->CommandBufferManager->GetOrCreateCommandBuffer();
    g_rhi_vk->m_swapchain_->GetSwapchainImage(FrameIndex)->CommandBufferFence
        = (VkFence)CommandBuffer->GetFenceHandle();
  }
}

}  // namespace game_engine
