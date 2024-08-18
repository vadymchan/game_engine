
#include "gfx/rhi/vulkan/render_frame_context_vk.h"

#include "gfx/rhi/vulkan/rhi_vk.h"

namespace game_engine {



void RenderFrameContextVk::SubmitCurrentActiveCommandBuffer(
    ECurrentRenderPass currentRenderPass) {
  SwapchainImageVk* swapchainImageVk
      = (SwapchainImageVk*)g_rhi_vk->m_swapchain_->GetSwapchainImage(m_frameIndex_);

  switch (currentRenderPass) {
    case RenderFrameContextVk::ShadowPass:
      QueueSubmitCurrentActiveCommandBuffer(
          swapchainImageVk->m_renderFinishedAfterShadow_);
      break;
    case RenderFrameContextVk::BasePass:
      QueueSubmitCurrentActiveCommandBuffer(
          swapchainImageVk->m_renderFinishedAfterBasePass_);
      break;
    default:
      break;
  }
}

void RenderFrameContextVk::QueueSubmitCurrentActiveCommandBuffer(
    Semaphore* InSignalSemaphore) {
  if (m_commandBuffer_) {
    // TODO: temoporary removed
    // m_commandBuffer->End();

    g_rhi_vk->QueueSubmit(shared_from_this(), InSignalSemaphore);
    g_rhi_vk->GetCommandBufferManager()->ReturnCommandBuffer(m_commandBuffer_);

    // get new command buffer
    m_commandBuffer_ = g_rhi_vk->m_commandBufferManager_->GetOrCreateCommandBuffer();
    g_rhi_vk->m_swapchain_->GetSwapchainImage(m_frameIndex_)->m_commandBufferFence_
        = (VkFence)m_commandBuffer_->GetFenceHandle();
  }
}

}  // namespace game_engine
