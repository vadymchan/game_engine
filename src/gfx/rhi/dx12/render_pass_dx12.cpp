#include "gfx/rhi/dx12/render_pass_dx12.h"
#include "gfx/rhi/dx12/texture_dx12.h"


namespace game_engine {

void RenderPassDx12::Release() {
  // if (m_frameBuffer)
  //{
  //     vkDestroyFramebuffer(g_rhi_vk->Device, m_frameBuffer, nullptr);
  //     m_frameBuffer = nullptr;
  // }

  // if (m_renderPass)
  //{
  //     vkDestroyRenderPass(g_rhi_vk->Device, m_renderPass, nullptr);
  //     m_renderPass = nullptr;
  // }
}

bool RenderPassDx12::BeginRenderPass(const CommandBuffer* commandBuffer) {
  bool isValidCommandBuffer = commandBuffer != nullptr;
  assert(isValidCommandBuffer);

  if (!isValidCommandBuffer) {
    return false;
  }


  m_commandBuffer_ = (const CommandBufferDx12*)commandBuffer;

  if (RTVCPUHandles.size() > 0) {
    m_commandBuffer_->CommandList->OMSetRenderTargets(
        (uint32_t)RTVCPUHandles.size(),
        &RTVCPUHandles[0],
        false,
        (DSVCPUDHandle.ptr ? &DSVCPUDHandle : nullptr));
  } else {
    m_commandBuffer_->CommandList->OMSetRenderTargets(
        0, nullptr, false, (DSVCPUDHandle.ptr ? &DSVCPUDHandle : nullptr));
  }

  for (int32_t i = 0; i < RTVClears.size(); ++i) {
    if (RTVClears[i].GetType() != ERTClearType::Color) {
      continue;
    }

    m_commandBuffer_->CommandList->ClearRenderTargetView(
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

      m_commandBuffer_->CommandList->ClearDepthStencilView(
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

void RenderPassDx12::EndRenderPass() {
  // assert(m_commandBuffer);

  //// Finishing up
  // vkCmdEndRenderPass((VkCommandBuffer)m_commandBuffer->GetHandle());

  //// Apply layout to attachments
  // for(Attachment& iter : m_renderPassInfo.Attachments)
  //{
  //     check(iter.IsValid());
  //     SetFinalLayoutToAttachment(iter);
  // }

  // m_commandBuffer = nullptr;
}

void RenderPassDx12::SetFinalLayoutToAttachment(
    const Attachment& attachment) const {
  // check(attachment.RenderTargetPtr);
  // TextureDx12* texture_vk =
  // (TextureDx12*)attachment.RenderTargetPtr->GetTexture();
  // texture_vk->Layout = attachment.FinalLayout;
}

void RenderPassDx12::Initialize() {
  CreateRenderPass();
}

bool RenderPassDx12::CreateRenderPass() {
  // Create m_renderPass
  {
    for (int32_t i = 0; i < (int32_t)m_renderPassInfo.Attachments.size(); ++i) {
      const Attachment& attachment = m_renderPassInfo.Attachments[i];
      assert(attachment.IsValid());

      const auto& RTInfo = attachment.RenderTargetPtr->Info;
      const bool  HasClear
          = (attachment.LoadStoreOp == EAttachmentLoadStoreOp::CLEAR_STORE
             || attachment.LoadStoreOp
                    == EAttachmentLoadStoreOp::CLEAR_DONTCARE);
      TextureDx12* TextureDX12
          = (TextureDx12*)attachment.RenderTargetPtr->GetTexture();

      if (attachment.IsDepthAttachment()) {
        DSVFormat = GetDX12TextureFormat(RTInfo.Format);
        DSVClear  = attachment.m_rtClearValue;

        DSVDepthClear   = HasClear;
        DSVStencilClear = (attachment.StencilLoadStoreOp
                               == EAttachmentLoadStoreOp::CLEAR_STORE
                           || attachment.StencilLoadStoreOp
                                  == EAttachmentLoadStoreOp::CLEAR_DONTCARE);

        DSVCPUDHandle = TextureDX12->DSV.CPUHandle;
      } else {
        if (HasClear) {
          RTVClears.push_back(attachment.m_rtClearValue);
        } else {
          RTVClears.push_back(RTClearValue::Invalid);
        }
        RTVCPUHandles.push_back(TextureDX12->RTV.CPUHandle);
        RTVFormats.push_back(GetDX12TextureFormat(RTInfo.Format));
      }
    }
  }

  return true;
}

}  // namespace game_engine