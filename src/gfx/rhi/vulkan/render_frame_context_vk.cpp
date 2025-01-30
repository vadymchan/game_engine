
#include "gfx/rhi/vulkan/render_frame_context_vk.h"

#include "gfx/rhi/vulkan/rhi_vk.h"

#include <memory>

namespace game_engine {

void RenderFrameContextVk::submitCurrentActiveCommandBuffer(
    ECurrentRenderPass currentRenderPass) {
  auto swapchainImageVk = std::static_pointer_cast<SwapchainImageVk>(
      g_rhiVk->m_swapchain_->getSwapchainImage(m_frameIndex_));

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
    g_rhiVk->queueSubmit(shared_from_this(), signalSemaphore);
    g_rhiVk->getCommandBufferManager()->returnCommandBuffer(m_commandBuffer_);

    // get new command buffer
    m_commandBuffer_
        = g_rhiVk->m_commandBufferManager_->getOrCreateCommandBuffer();

    auto swapchain        = g_rhiVk->m_swapchain_;
    auto swapchainImageVk = std::static_pointer_cast<SwapchainImageVk>(
        swapchain->getSwapchainImage(m_frameIndex_));
    auto* swapchainFence = swapchainImageVk->m_commandBufferFence_;
    swapchainFence       = (VkFence)m_commandBuffer_->getFenceHandle();
  }
}

}  // namespace game_engine
