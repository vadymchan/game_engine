
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
    Semaphore* InSignalSemaphore) {
  if (m_commandBuffer_) {
    // TODO: temoporary removed
    // m_commandBuffer_->End();

    g_rhi_vk->QueueSubmit(shared_from_this(), InSignalSemaphore);
    g_rhi_vk->GetCommandBufferManager()->ReturnCommandBuffer(m_commandBuffer_);

    // get new command buffer
    m_commandBuffer_ = g_rhi_vk->m_commandBufferManager_->GetOrCreateCommandBuffer();
    g_rhi_vk->m_swapchain_->GetSwapchainImage(FrameIndex)->CommandBufferFence
        = (VkFence)m_commandBuffer_->GetFenceHandle();
  }
}

}  // namespace game_engine
