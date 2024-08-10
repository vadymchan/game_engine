#include "gfx/rhi/dx12/render_frame_context_dx12.h"
#include "gfx/rhi/dx12/rhi_dx12.h"
#include "gfx/rhi/dx12/command_allocator_dx12.h"


namespace game_engine {

void jRenderFrameContext_DX12::QueueSubmitCurrentActiveCommandBuffer() {
  if (CommandBuffer) {
    jCommandBuffer_DX12* CommandBuffer_DX12
        = (jCommandBuffer_DX12*)CommandBuffer;

    auto CommandBufferManager = g_rhi_dx12->GetCommandBufferManager();
    CommandBufferManager->ExecuteCommandList(CommandBuffer_DX12);
    CommandBufferManager->ReturnCommandBuffer(CommandBuffer_DX12);

    // get new commandbuffer
    CommandBuffer = CommandBufferManager->GetOrCreateCommandBuffer();
    g_rhi_dx12->Swapchain->Images[FrameIndex]->FenceValue
        = CommandBuffer_DX12->Owner->Fence->SignalWithNextFenceValue(
            CommandBuffer_DX12->Owner->GetCommandQueue().Get());
  }
}

void jRenderFrameContext_DX12::SubmitCurrentActiveCommandBuffer(
    ECurrentRenderPass InCurrentRenderPass) {
  if (CommandBuffer) {
    jCommandBuffer_DX12* CommandBuffer_DX12
        = (jCommandBuffer_DX12*)CommandBuffer;

    auto CommandBufferManager = g_rhi_dx12->GetCommandBufferManager();
    CommandBufferManager->ExecuteCommandList(CommandBuffer_DX12);
    CommandBufferManager->ReturnCommandBuffer(CommandBuffer_DX12);

    // get new commandbuffer
    CommandBuffer = CommandBufferManager->GetOrCreateCommandBuffer();
    g_rhi_dx12->Swapchain->Images[FrameIndex]->FenceValue
        = CommandBuffer_DX12->Owner->Fence->SignalWithNextFenceValue(
            CommandBuffer_DX12->Owner->GetCommandQueue().Get());
  }
}

}  // namespace game_engine