#include "gfx/rhi/dx12/render_frame_context_dx12.h"
#include "gfx/rhi/dx12/rhi_dx12.h"
#include "gfx/rhi/dx12/command_allocator_dx12.h"


namespace game_engine {

void RenderFrameContextDx12::queueSubmitCurrentActiveCommandBuffer() {
  if (m_commandBuffer_) {
    CommandBufferDx12* commandBufferDx12
        = (CommandBufferDx12*)m_commandBuffer_;

    auto commandBufferManager = g_rhi_dx12->getCommandBufferManager();
    commandBufferManager->executeCommandList(commandBufferDx12);
    commandBufferManager->returnCommandBuffer(commandBufferDx12);

    // get new commandbuffer
    m_commandBuffer_ = commandBufferManager->getOrCreateCommandBuffer();
    g_rhi_dx12->m_swapchain_->m_images_[m_frameIndex_]->m_fenceValue_
        = commandBufferDx12->m_owner_->m_fence->signalWithNextFenceValue(
            commandBufferDx12->m_owner_->getCommandQueue().Get());
  }
}

void RenderFrameContextDx12::submitCurrentActiveCommandBuffer(
    ECurrentRenderPass currentRenderPass) {
  if (m_commandBuffer_) {
    CommandBufferDx12* commandBufferDx12
        = (CommandBufferDx12*)m_commandBuffer_;

    auto commandBufferManager = g_rhi_dx12->getCommandBufferManager();
    commandBufferManager->executeCommandList(commandBufferDx12);
    commandBufferManager->returnCommandBuffer(commandBufferDx12);

    // get new commandbuffer
    m_commandBuffer_ = commandBufferManager->getOrCreateCommandBuffer();
    g_rhi_dx12->m_swapchain_->m_images_[m_frameIndex_]->m_fenceValue_
        = commandBufferDx12->m_owner_->m_fence->signalWithNextFenceValue(
            commandBufferDx12->m_owner_->getCommandQueue().Get());
  }
}

}  // namespace game_engine