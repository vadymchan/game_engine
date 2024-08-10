#include "gfx/rhi/dx12/render_pass_dx12.h"
#include "gfx/rhi/dx12/texture_dx12.h"


namespace game_engine {

void jRenderPass_DX12::Release() {
  // if (FrameBuffer)
  //{
  //     vkDestroyFramebuffer(g_rhi_vk->Device, FrameBuffer, nullptr);
  //     FrameBuffer = nullptr;
  // }

  // if (RenderPass)
  //{
  //     vkDestroyRenderPass(g_rhi_vk->Device, RenderPass, nullptr);
  //     RenderPass = nullptr;
  // }
}

bool jRenderPass_DX12::BeginRenderPass(const jCommandBuffer* commandBuffer) {
  bool isValidCommandBuffer = commandBuffer != nullptr;
  assert(isValidCommandBuffer);

  if (!isValidCommandBuffer) {
    return false;
  }


  CommandBuffer = (const jCommandBuffer_DX12*)commandBuffer;

  if (RTVCPUHandles.size() > 0) {
    CommandBuffer->CommandList->OMSetRenderTargets(
        (uint32_t)RTVCPUHandles.size(),
        &RTVCPUHandles[0],
        false,
        (DSVCPUDHandle.ptr ? &DSVCPUDHandle : nullptr));
  } else {
    CommandBuffer->CommandList->OMSetRenderTargets(
        0, nullptr, false, (DSVCPUDHandle.ptr ? &DSVCPUDHandle : nullptr));
  }

  for (int32_t i = 0; i < RTVClears.size(); ++i) {
    if (RTVClears[i].GetType() != ERTClearType::Color) {
      continue;
    }

    CommandBuffer->CommandList->ClearRenderTargetView(
        RTVCPUHandles[i], RTVClears[i].GetCleraColor(), 0, nullptr);
  }

  if (DSVClear.GetType() == ERTClearType::DepthStencil) {
    if (DSVDepthClear || DSVStencilClear) {
      D3D12_CLEAR_FLAGS DSVClearFlags = (D3D12_CLEAR_FLAGS)0;
      if (DSVDepthClear) {
        DSVClearFlags |= D3D12_CLEAR_FLAG_DEPTH;
      }

      if (DSVStencilClear) {
        DSVClearFlags |= D3D12_CLEAR_FLAG_STENCIL;
      }

      CommandBuffer->CommandList->ClearDepthStencilView(
          DSVCPUDHandle,
          DSVClearFlags,
          DSVClear.GetCleraDepth(),
          (uint8_t)DSVClear.GetCleraStencil(),
          0,
          nullptr);
    }
  }

  return true;
}

void jRenderPass_DX12::EndRenderPass() {
  // assert(CommandBuffer);

  //// Finishing up
  // vkCmdEndRenderPass((VkCommandBuffer)CommandBuffer->GetHandle());

  //// Apply layout to attachments
  // for(jAttachment& iter : RenderPassInfo.Attachments)
  //{
  //     check(iter.IsValid());
  //     SetFinalLayoutToAttachment(iter);
  // }

  // CommandBuffer = nullptr;
}

void jRenderPass_DX12::SetFinalLayoutToAttachment(
    const jAttachment& attachment) const {
  // check(attachment.RenderTargetPtr);
  // jTexture_DX12* texture_vk =
  // (jTexture_DX12*)attachment.RenderTargetPtr->GetTexture();
  // texture_vk->Layout = attachment.FinalLayout;
}

void jRenderPass_DX12::Initialize() {
  CreateRenderPass();
}

bool jRenderPass_DX12::CreateRenderPass() {
  // Create RenderPass
  {
    for (int32_t i = 0; i < (int32_t)RenderPassInfo.Attachments.size(); ++i) {
      const jAttachment& attachment = RenderPassInfo.Attachments[i];
      assert(attachment.IsValid());

      const auto& RTInfo = attachment.RenderTargetPtr->Info;
      const bool  HasClear
          = (attachment.LoadStoreOp == EAttachmentLoadStoreOp::CLEAR_STORE
             || attachment.LoadStoreOp
                    == EAttachmentLoadStoreOp::CLEAR_DONTCARE);
      jTexture_DX12* TextureDX12
          = (jTexture_DX12*)attachment.RenderTargetPtr->GetTexture();

      if (attachment.IsDepthAttachment()) {
        DSVFormat = GetDX12TextureFormat(RTInfo.Format);
        DSVClear  = attachment.RTClearValue;

        DSVDepthClear   = HasClear;
        DSVStencilClear = (attachment.StencilLoadStoreOp
                               == EAttachmentLoadStoreOp::CLEAR_STORE
                           || attachment.StencilLoadStoreOp
                                  == EAttachmentLoadStoreOp::CLEAR_DONTCARE);

        DSVCPUDHandle = TextureDX12->DSV.CPUHandle;
      } else {
        if (HasClear) {
          RTVClears.push_back(attachment.RTClearValue);
        } else {
          RTVClears.push_back(jRTClearValue::Invalid);
        }
        RTVCPUHandles.push_back(TextureDX12->RTV.CPUHandle);
        RTVFormats.push_back(GetDX12TextureFormat(RTInfo.Format));
      }
    }
  }

  return true;
}

}  // namespace game_engine