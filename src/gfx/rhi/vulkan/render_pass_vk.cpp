
#include "gfx/rhi/vulkan/render_pass_vk.h"

#include "gfx/rhi/vulkan/rhi_vk.h"

namespace game_engine {

void RenderPassVk::Release() {
  if (m_frameBuffer_ != VK_NULL_HANDLE) {
    vkDestroyFramebuffer(g_rhi_vk->m_device_, m_frameBuffer_, nullptr);
    m_frameBuffer_ = VK_NULL_HANDLE;
  }
  if (m_renderPass_ != VK_NULL_HANDLE) {
    vkDestroyRenderPass(g_rhi_vk->m_device_, m_renderPass_, nullptr);
    m_frameBuffer_ = VK_NULL_HANDLE;
  }
}

void RenderPassVk::SetFinalLayoutToAttachment(
    const Attachment& attachment) const {
  assert(attachment.m_renderTargetPtr_);
  TextureVk* texture_vk = (TextureVk*)attachment.m_renderTargetPtr_->GetTexture();
  texture_vk->m_imageLayout_ = attachment.m_finalLayout_;
}

bool RenderPassVk::BeginRenderPass(const CommandBuffer* commandBuffer
                                   /*, VkSubpassContents      subpassContents*/
                                   ) {
  assert(commandBuffer);

  m_commandBuffer_ = commandBuffer;

  assert(m_frameBuffer_);
  m_renderPassBeginInfo_.framebuffer = m_frameBuffer_;

  vkCmdBeginRenderPass((VkCommandBuffer)commandBuffer->GetNativeHandle(),
                       &m_renderPassBeginInfo_,
                       /*subpassContents*/ VK_SUBPASS_CONTENTS_INLINE);
  return true;
}

void RenderPassVk::EndRenderPass() {
  assert(m_commandBuffer_);

  // Finishing up
  vkCmdEndRenderPass((VkCommandBuffer)m_commandBuffer_->GetNativeHandle());

  // Apply layout to attachments
  for (Attachment& iter : m_renderPassInfo_.m_attachments_) {
    assert(iter.IsValid());
    SetFinalLayoutToAttachment(iter);
  }

  m_commandBuffer_ = nullptr;
}

bool RenderPassVk::CreateRenderPass() {
  //  std::vector<VkAttachmentDescription> attachmentDescs;
  //  attachmentDescs.reserve(m_renderPassInfo_.Attachments.size());

  //  for (const auto& attachment : m_renderPassInfo_.Attachments) {
  //    VkAttachmentDescription desc = {};
  //    desc.format                  =
  //    attachment.RenderTargetPtr->Info.Format; desc.samples        =
  //    VK_SAMPLE_COUNT_1_BIT;  // TODO: remove hardcode desc.loadOp         =
  //    attachment.LoadOp; desc.storeOp        = attachment.StoreOp;
  //    desc.stencilLoadOp  = attachment.StencilLoadOp;
  //    desc.stencilStoreOp = attachment.StencilStoreOp;
  //    desc.initialLayout  = attachment.InitialLayout;
  //    desc.finalLayout    = attachment.FinalLayout;
  //    attachmentDescs.push_back(desc);
  //  }

  //  // Subpasses and dependencies setup based on m_renderPassInfo_
  //  // ...

  //  VkRenderPassCreateInfo renderPassInfo = {};
  //  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  //  renderPassInfo.attachmentCount
  //      = static_cast<uint32_t>(attachmentDescs.size());
  //  renderPassInfo.pAttachments = attachmentDescs.data();
  //  // Include subpasses and dependencies based on m_renderPassInfo_
  //  // ...

  //  return vkCreateRenderPass(
  //             g_rhi_vk->m_device_, &renderPassInfo, nullptr, &renderPass)
  //      == VK_SUCCESS;
  //}

  int32_t SampleCount = 0;
  int32_t LayerCount  = 0;

  // Create RenderPass
  {
    std::vector<VkAttachmentDescription> AttachmentDescs;
    AttachmentDescs.resize(m_renderPassInfo_.m_attachments_.size());
    for (int32_t i = 0; i < (int32_t)m_renderPassInfo_.m_attachments_.size(); ++i) {
      const Attachment& attachment = m_renderPassInfo_.m_attachments_[i];
      assert(attachment.IsValid());

      const auto& RTInfo = attachment.m_renderTargetPtr_->m_info_;

      VkAttachmentDescription& attachmentDesc = AttachmentDescs[i];
      attachmentDesc.format  = GetVulkanTextureFormat(RTInfo.m_format_);
      attachmentDesc.samples = (VkSampleCountFlagBits)RTInfo.m_sampleCount_;
      GetVulkanAttachmentLoadStoreOp(attachmentDesc.loadOp,
                                     attachmentDesc.storeOp,
                                     attachment.m_loadStoreOp_);
      GetVulkanAttachmentLoadStoreOp(attachmentDesc.stencilLoadOp,
                                     attachmentDesc.stencilStoreOp,
                                     attachment.m_stencilLoadStoreOp_);

      assert((attachment.IsResolveAttachment()
              && ((int32_t)RTInfo.m_sampleCount_ > 1))
          || (!attachment.IsResolveAttachment()
              && (int32_t)RTInfo.m_sampleCount_ == 1));
      const bool IsInvalidSampleCountAndLayerCount
          = (SampleCount == 0) || (LayerCount == 0);
      if (!attachment.IsDepthAttachment()
          || IsInvalidSampleCountAndLayerCount) {
        assert(SampleCount == 0 || SampleCount == (int32_t)RTInfo.m_sampleCount_);
        assert(LayerCount == 0 || LayerCount == RTInfo.m_layerCount_);
        SampleCount = (int32_t)RTInfo.m_sampleCount_;
        LayerCount  = RTInfo.m_layerCount_;
      }
      attachmentDesc.initialLayout
          = GetVulkanImageLayout(attachment.m_initialLayout_);
      attachmentDesc.finalLayout = GetVulkanImageLayout(attachment.m_finalLayout_);

      const auto&  RTClearColor = attachment.m_rtClearValue.GetCleraColor();
      VkClearValue clearValue   = {};
      if (attachment.IsDepthAttachment()) {
        clearValue.depthStencil = {attachment.m_rtClearValue.GetCleraDepth(),
                                   attachment.m_rtClearValue.GetCleraStencil()};
      } else {
        clearValue.color = {
          RTClearColor[0], RTClearColor[1], RTClearColor[2], RTClearColor[3]};
      }
      m_clearValues_.push_back(clearValue);
    }

    struct SubpassAttachmentRefs {
      std::vector<VkAttachmentReference>   m_inputAttachmentRefs;
      std::vector<VkAttachmentReference>   m_outputColorAttachmentRefs;
      std::optional<VkAttachmentReference> m_outputDepthAttachmentRef;
      std::optional<VkAttachmentReference> m_outputResolveAttachmentRef;
    };

    assert(m_renderPassInfo_.m_subpasses_.size());

    std::vector<SubpassAttachmentRefs> subpassAttachmentRefs;
    subpassAttachmentRefs.resize(m_renderPassInfo_.m_subpasses_.size());

    std::vector<VkSubpassDescription> SubpassDescs;
    SubpassDescs.resize(m_renderPassInfo_.m_subpasses_.size());

    std::vector<VkSubpassDependency> SubpassDependencies;
    SubpassDependencies.resize(m_renderPassInfo_.m_subpasses_.size() + 1);

    const bool IsSubpassForExecuteInOrder
        = m_renderPassInfo_.IsSubpassForExecuteInOrder();

    int32_t DependencyIndex                         = 0;
    SubpassDependencies[DependencyIndex].srcSubpass = VK_SUBPASS_EXTERNAL;
    SubpassDependencies[DependencyIndex].dstSubpass = 0;
    SubpassDependencies[DependencyIndex].srcStageMask
        = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    SubpassDependencies[DependencyIndex].dstStageMask
        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    SubpassDependencies[DependencyIndex].srcAccessMask
        = VK_ACCESS_MEMORY_READ_BIT;
    SubpassDependencies[DependencyIndex].dstAccessMask
        = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT
        | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    SubpassDependencies[DependencyIndex].dependencyFlags
        = VK_DEPENDENCY_BY_REGION_BIT;
    ++DependencyIndex;

    for (int32_t i = 0; i < (int32_t)m_renderPassInfo_.m_subpasses_.size(); ++i) {
      const Subpass& subPass = m_renderPassInfo_.m_subpasses_[i];

      std::vector<VkAttachmentReference>& InputAttachmentRefs
          = subpassAttachmentRefs[i].m_inputAttachmentRefs;
      std::vector<VkAttachmentReference>& OutputColorAttachmentRefs
          = subpassAttachmentRefs[i].m_outputColorAttachmentRefs;
      std::optional<VkAttachmentReference>& OutputDepthAttachmentRef
          = subpassAttachmentRefs[i].m_outputDepthAttachmentRef;
      std::optional<VkAttachmentReference>& OutputResolveAttachmentRef
          = subpassAttachmentRefs[i].m_outputResolveAttachmentRef;

      for (const int32_t& attachmentIndex : subPass.m_inputAttachments_) {
        const VkAttachmentReference attchmentRef = {
          (uint32_t)attachmentIndex, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
        InputAttachmentRefs.emplace_back(attchmentRef);
      }
      for (const int32_t& attachmentIndex : subPass.m_outputColorAttachments_) {
        const VkAttachmentReference attchmentRef = {
          (uint32_t)attachmentIndex, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
        OutputColorAttachmentRefs.emplace_back(attchmentRef);
      }
      if (subPass.m_outputDepthAttachment_) {
        const VkAttachmentReference attchmentRef
            = {(uint32_t)subPass.m_outputDepthAttachment_.value(),
               VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
        OutputDepthAttachmentRef = attchmentRef;
      }
      if (subPass.m_outputResolveAttachment_) {
        const VkAttachmentReference attchmentRef
            = {(uint32_t)subPass.m_outputDepthAttachment_.value(),
               VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
        OutputResolveAttachmentRef = attchmentRef;
      }

      SubpassDescs[i].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
      if (InputAttachmentRefs.size() > 0) {
        SubpassDescs[i].inputAttachmentCount
            = (uint32_t)InputAttachmentRefs.size();
        SubpassDescs[i].pInputAttachments = InputAttachmentRefs.data();
      }
      if (OutputColorAttachmentRefs.size() > 0) {
        SubpassDescs[i].colorAttachmentCount
            = (uint32_t)OutputColorAttachmentRefs.size();
        SubpassDescs[i].pColorAttachments = OutputColorAttachmentRefs.data();
      }
      if (OutputDepthAttachmentRef) {
        SubpassDescs[i].pDepthStencilAttachment
            = &OutputDepthAttachmentRef.value();
      }
      if (OutputResolveAttachmentRef) {
        SubpassDescs[i].pResolveAttachments
            = &OutputResolveAttachmentRef.value();
      }

      // This dependency transitions the input attachment from color
      // attachment to shader read
      if (IsSubpassForExecuteInOrder) {
        SubpassDependencies[DependencyIndex].srcSubpass = (uint32_t)(i);
        SubpassDependencies[DependencyIndex].dstSubpass = (uint32_t)(i + 1);
      } else {
        SubpassDependencies[DependencyIndex].srcSubpass
            = (uint32_t)subPass.m_sourceSubpassIndex_;
        SubpassDependencies[DependencyIndex].dstSubpass
            = (uint32_t)subPass.m_destSubpassIndex_;
      }
      SubpassDependencies[DependencyIndex].srcStageMask
          = GetPipelineStageMask(subPass.m_attachmentProducePipelineBit_);
      SubpassDependencies[DependencyIndex].dstStageMask
          = GetPipelineStageMask(subPass.m_attachmentConsumePipelineBit_);
      SubpassDependencies[DependencyIndex].srcAccessMask
          = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
      SubpassDependencies[DependencyIndex].dstAccessMask
          = VK_ACCESS_SHADER_READ_BIT;
      SubpassDependencies[DependencyIndex].dependencyFlags
          = VK_DEPENDENCY_BY_REGION_BIT;
      ++DependencyIndex;
    }

    --DependencyIndex;
    SubpassDependencies[DependencyIndex].srcSubpass = 0;
    SubpassDependencies[DependencyIndex].dstSubpass = VK_SUBPASS_EXTERNAL;
    SubpassDependencies[DependencyIndex].srcStageMask
        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    SubpassDependencies[DependencyIndex].dstStageMask
        = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    SubpassDependencies[DependencyIndex].srcAccessMask
        = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT
        | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    SubpassDependencies[DependencyIndex].dstAccessMask
        = VK_ACCESS_MEMORY_READ_BIT;
    SubpassDependencies[DependencyIndex].dependencyFlags
        = VK_DEPENDENCY_BY_REGION_BIT;

    VkRenderPassCreateInfo renderPassCreateInfo = {};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.attachmentCount
        = static_cast<uint32_t>(AttachmentDescs.size());
    renderPassCreateInfo.pAttachments    = AttachmentDescs.data();
    renderPassCreateInfo.subpassCount    = (uint32_t)SubpassDescs.size();
    renderPassCreateInfo.pSubpasses      = SubpassDescs.data();
    renderPassCreateInfo.dependencyCount = (uint32_t)SubpassDependencies.size();
    renderPassCreateInfo.pDependencies   = SubpassDependencies.data();

    if (vkCreateRenderPass(
            g_rhi_vk->m_device_, &renderPassCreateInfo, nullptr, &m_renderPass_)
        != VK_SUCCESS) {
      return false;
    }
  }

  // Create RenderPassBeginInfo
  m_renderPassBeginInfo_            = {};
  m_renderPassBeginInfo_.sType      = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  m_renderPassBeginInfo_.renderPass = m_renderPass_;

  m_renderPassBeginInfo_.renderArea.offset = {m_renderOffset_.x(), m_renderOffset_.y()};
  m_renderPassBeginInfo_.renderArea.extent
      = {(uint32_t)m_renderExtent_.x(), (uint32_t)m_renderExtent_.y()};

  m_renderPassBeginInfo_.clearValueCount
      = static_cast<uint32_t>(m_clearValues_.size());
  m_renderPassBeginInfo_.pClearValues = m_clearValues_.data();

  // Create framebuffer
  {
    std::vector<VkImageView> ImageViews;

    for (int32_t k = 0; k < m_renderPassInfo_.m_attachments_.size(); ++k) {
      assert(m_renderPassInfo_.m_attachments_[k].IsValid());

      const auto* RT = m_renderPassInfo_.m_attachments_[k].m_renderTargetPtr_.get();
      assert(RT);

      const TextureVk* texture_vk = (const TextureVk*)RT->GetTexture();
      assert(texture_vk);

      ImageViews.push_back(texture_vk->m_imageView_);
    }

    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType      = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = m_renderPass_;

    framebufferInfo.attachmentCount = static_cast<uint32_t>(ImageViews.size());
    framebufferInfo.pAttachments    = ImageViews.data();

    framebufferInfo.width  = m_renderExtent_.x();
    framebufferInfo.height = m_renderExtent_.y();
    framebufferInfo.layers = LayerCount;

    if (vkCreateFramebuffer(
            g_rhi_vk->m_device_, &framebufferInfo, nullptr, &m_frameBuffer_)
        != VK_SUCCESS) {
      return false;
    }
  }

  return true;
}

}  // namespace game_engine
