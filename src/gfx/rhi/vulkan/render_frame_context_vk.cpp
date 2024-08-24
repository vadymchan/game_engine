
#include "gfx/rhi/vulkan/render_frame_context_vk.h"

#include "gfx/rhi/vulkan/rhi_vk.h"

namespace game_engine {



void RenderFrameContextVk::submitCurrentActiveCommandBuffer(
    ECurrentRenderPass currentRenderPass) {
  SwapchainImageVk* swapchainImageVk
      = (SwapchainImageVk*)g_rhi_vk->m_swapchain_->getSwapchainImage(m_frameIndex_);

  switch (currentRenderPass) {
    case RenderFrameContextVk::ShadowPass:
      queueSubmitCurrentActiveCommandBuffer(
          swapchainImageVk->m_renderFinishedAfterShadow_);
      break;
    case RenderFrameContextVk::BasePass:
      queueSubmitCurrentActiveCommandBuffer(
          swapchainImageVk->m_renderFinishedAfterBasePass_);
      break;
    default:
      break;
  }
}

void RenderFrameContextVk::queueSubmitCurrentActiveCommandBuffer(
    ISemaphore* signalSemaphore) {
  if (m_commandBuffer_) {
    // TODO: temoporary removed
    // m_commandBuffer->end();

    g_rhi_vk->queueSubmit(shared_from_this(), signalSemaphore);
    g_rhi_vk->getCommandBufferManager()->returnCommandBuffer(m_commandBuffer_);

    // get new command buffer
    m_commandBuffer_ = g_rhi_vk->m_commandBufferManager_->getOrCreateCommandBuffer();
    g_rhi_vk->m_swapchain_->getSwapchainImage(m_frameIndex_)->m_commandBufferFence_
        = (VkFence)m_commandBuffer_->getFenceHandle();
  }
}

}  // namespace game_engine
