#include "gfx/rhi/dx12/render_frame_context_dx12.h"
#include "gfx/rhi/dx12/rhi_dx12.h"
#include "gfx/rhi/dx12/command_allocator_dx12.h"


namespace game_engine {

void RenderFrameContextDx12::QueueSubmitCurrentActiveCommandBuffer() {
  if (m_commandBuffer) {
    CommandBufferDx12* commandBufferDx12
        = (CommandBufferDx12*)m_commandBuffer;

    auto commandBufferManager = g_rhi_dx12->GetCommandBufferManager();
    commandBufferManager->ExecuteCommandList(commandBufferDx12);
    commandBufferManager->ReturnCommandBuffer(commandBufferDx12);

    // get new commandbuffer
    m_commandBuffer = commandBufferManager->GetOrCreateCommandBuffer();
    g_rhi_dx12->m_swapchain_->Images[FrameIndex]->FenceValue
        = commandBufferDx12->Owner->m_fence->SignalWithNextFenceValue(
            commandBufferDx12->Owner->GetCommandQueue().Get());
  }
}

void RenderFrameContextDx12::SubmitCurrentActiveCommandBuffer(
    ECurrentRenderPass InCurrentRenderPass) {
  if (m_commandBuffer) {
    CommandBufferDx12* commandBufferDx12
        = (CommandBufferDx12*)m_commandBuffer;

    auto commandBufferManager = g_rhi_dx12->GetCommandBufferManager();
    commandBufferManager->ExecuteCommandList(commandBufferDx12);
    commandBufferManager->ReturnCommandBuffer(commandBufferDx12);

    // get new commandbuffer
    m_commandBuffer = commandBufferManager->GetOrCreateCommandBuffer();
    g_rhi_dx12->m_swapchain_->Images[FrameIndex]->FenceValue
        = commandBufferDx12->Owner->m_fence->SignalWithNextFenceValue(
            commandBufferDx12->Owner->GetCommandQueue().Get());
  }
}

}  // namespace game_engine