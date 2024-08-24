#include "gfx/rhi/dx12/render_pass_dx12.h"
#include "gfx/rhi/dx12/texture_dx12.h"


namespace game_engine {

void RenderPassDx12::release() {
  // if (m_frameBuffer)
  //{
  //     vkDestroyFramebuffer(g_rhiVk->Device, m_frameBuffer, nullptr);
  //     m_frameBuffer = nullptr;
  // }

  // if (kRenderPass)
  //{
  //     vkDestroyRenderPass(g_rhiVk->Device, kRenderPass, nullptr);
  //     kRenderPass = nullptr;
  // }
}

bool RenderPassDx12::beginRenderPass(const CommandBuffer* commandBuffer) {
  bool isValidCommandBuffer = commandBuffer != nullptr;
  assert(isValidCommandBuffer);

  if (!isValidCommandBuffer) {
    return false;
  }


  m_commandBuffer_ = (const CommandBufferDx12*)commandBuffer;

  if (m_rtvCPUHandles_.size() > 0) {
    m_commandBuffer_->m_commandList_->OMSetRenderTargets(
        (uint32_t)m_rtvCPUHandles_.size(),
        &m_rtvCPUHandles_[0],
        false,
        (m_dsvCPUDHandle_.ptr ? &m_dsvCPUDHandle_ : nullptr));
  } else {
    m_commandBuffer_->m_commandList_->OMSetRenderTargets(
        0, nullptr, false, (m_dsvCPUDHandle_.ptr ? &m_dsvCPUDHandle_ : nullptr));
  }

  for (int32_t i = 0; i < m_rtvClears_.size(); ++i) {
    if (m_rtvClears_[i].getType() != ERTClearType::Color) {
      continue;
    }

    m_commandBuffer_->m_commandList_->ClearRenderTargetView(
        m_rtvCPUHandles_[i], m_rtvClears_[i].getCleraColor(), 0, nullptr);
  }

  if (m_dsvClear_.getType() == ERTClearType::DepthStencil) {
    if (m_dsvDepthClear_ || m_dsvStencilClear_) {
      D3D12_CLEAR_FLAGS DSVClearFlags = (D3D12_CLEAR_FLAGS)0;
      if (m_dsvDepthClear_) {
        DSVClearFlags |= D3D12_CLEAR_FLAG_DEPTH;
      }

      if (m_dsvStencilClear_) {
        DSVClearFlags |= D3D12_CLEAR_FLAG_STENCIL;
      }

      m_commandBuffer_->m_commandList_->ClearDepthStencilView(
          m_dsvCPUDHandle_,
          DSVClearFlags,
          m_dsvClear_.getClearDepth(),
          (uint8_t)m_dsvClear_.getClearStencil(),
          0,
          nullptr);
    }
  }

  return true;
}

void RenderPassDx12::endRenderPass() {
  // assert(m_commandBuffer);

  //// Finishing up
  // vkCmdEndRenderPass((VkCommandBuffer)m_commandBuffer->getHandle());

  //// Apply layout to attachments
  // for(Attachment& iter : m_renderPassInfo_.Attachments)
  //{
  //     check(iter.isValid());
  //     setFinalLayoutToAttachment_(iter);
  // }

  // m_commandBuffer = nullptr;
}

void RenderPassDx12::setFinalLayoutToAttachment_(
    const Attachment& attachment) const {
  // check(attachment.RenderTargetPtr);
  // TextureDx12* texture_vk =
  // (TextureDx12*)attachment.RenderTargetPtr->getTexture();
  // texture_vk->Layout = attachment.FinalLayout;
}

void RenderPassDx12::initialize() {
  createRenderPass();
}

bool RenderPassDx12::createRenderPass() {
  // Create kRenderPass
  {
    for (int32_t i = 0; i < (int32_t)m_renderPassInfo_.m_attachments_.size(); ++i) {
      const Attachment& attachment = m_renderPassInfo_.m_attachments_[i];
      assert(attachment.isValid());

      const auto& RTInfo = attachment.m_renderTargetPtr_->m_info_;
      const bool  HasClear
          = (attachment.m_loadStoreOp_ == EAttachmentLoadStoreOp::CLEAR_STORE
             || attachment.m_loadStoreOp_
                    == EAttachmentLoadStoreOp::CLEAR_DONTCARE);
      TextureDx12* TextureDX12
          = (TextureDx12*)attachment.m_renderTargetPtr_->getTexture();

      if (attachment.isDepthAttachment()) {
        m_dsvFormat_ = g_getDX12TextureFormat(RTInfo.m_format_);
        m_dsvClear_  = attachment.m_rtClearValue;

        m_dsvDepthClear_   = HasClear;
        m_dsvStencilClear_ = (attachment.m_stencilLoadStoreOp_
                               == EAttachmentLoadStoreOp::CLEAR_STORE
                           || attachment.m_stencilLoadStoreOp_
                                  == EAttachmentLoadStoreOp::CLEAR_DONTCARE);

        m_dsvCPUDHandle_ = TextureDX12->m_dsv_.m_cpuHandle_;
      } else {
        if (HasClear) {
          m_rtvClears_.push_back(attachment.m_rtClearValue);
        } else {
          m_rtvClears_.push_back(RTClearValue::s_kInvalid);
        }
        m_rtvCPUHandles_.push_back(TextureDX12->m_rtv_.m_cpuHandle_);
        m_rtvFormats_.push_back(g_getDX12TextureFormat(RTInfo.m_format_));
      }
    }
  }

  return true;
}

}  // namespace game_engine