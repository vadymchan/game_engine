
#include "gfx/rhi/vulkan/render_pass_vk.h"

#include "gfx/rhi/vulkan/rhi_vk.h"

namespace game_engine {

void RenderPassVk::Release() {
  if (m_frameBuffer != VK_NULL_HANDLE) {
    vkDestroyFramebuffer(g_rhi_vk->m_device_, m_frameBuffer, nullptr);
    m_frameBuffer = VK_NULL_HANDLE;
  }
  if (m_renderPass != VK_NULL_HANDLE) {
    vkDestroyRenderPass(g_rhi_vk->m_device_, m_renderPass, nullptr);
    m_frameBuffer = VK_NULL_HANDLE;
  }
}

void RenderPassVk::SetFinalLayoutToAttachment(
    const Attachment& attachment) const {
  assert(attachment.RenderTargetPtr);
  TextureVk* texture_vk = (TextureVk*)attachment.RenderTargetPtr->GetTexture();
  texture_vk->imageLayout = attachment.FinalLayout;
}

bool RenderPassVk::BeginRenderPass(const CommandBuffer* commandBuffer
                                   /*, VkSubpassContents      subpassContents*/
                                   ) {
  assert(commandBuffer);

  m_CommandBuffer = commandBuffer;

  assert(m_frameBuffer);
  RenderPassBeginInfo.framebuffer = m_frameBuffer;

  vkCmdBeginRenderPass((VkCommandBuffer)commandBuffer->GetNativeHandle(),
                       &RenderPassBeginInfo,
                       /*subpassContents*/ VK_SUBPASS_CONTENTS_INLINE);
  return true;
}

void RenderPassVk::EndRenderPass() {
  assert(m_CommandBuffer);

  // Finishing up
  vkCmdEndRenderPass((VkCommandBuffer)m_CommandBuffer->GetNativeHandle());

  // Apply layout to attachments
  for (Attachment& iter : m_renderPassInfo.Attachments) {
    assert(iter.IsValid());
    SetFinalLayoutToAttachment(iter);
  }

  m_CommandBuffer = nullptr;
}

bool RenderPassVk::CreateRenderPass() {
  //  std::vector<VkAttachmentDescription> attachmentDescs;
  //  attachmentDescs.reserve(m_renderPassInfo.Attachments.size());

  //  for (const auto& attachment : m_renderPassInfo.Attachments) {
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

  //  // Subpasses and dependencies setup based on RenderPassInfo
  //  // ...

  //  VkRenderPassCreateInfo renderPassInfo = {};
  //  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  //  renderPassInfo.attachmentCount
  //      = static_cast<uint32_t>(attachmentDescs.size());
  //  renderPassInfo.pAttachments = attachmentDescs.data();
  //  // Include subpasses and dependencies based on RenderPassInfo
  //  // ...

  //  return vkCreateRenderPass(
  //             g_rhi_vk->m_device_, &renderPassInfo, nullptr, &renderPass)
  //      == VK_SUCCESS;
  //}

  int32_t SampleCount = 0;
  int32_t LayerCount  = 0;

  // Create m_renderPass
  {
    std::vector<VkAttachmentDescription> AttachmentDescs;
    AttachmentDescs.resize(m_renderPassInfo.Attachments.size());
    for (int32_t i = 0; i < (int32_t)m_renderPassInfo.Attachments.size(); ++i) {
      const Attachment& attachment = m_renderPassInfo.Attachments[i];
      assert(attachment.IsValid());

      const auto& RTInfo = attachment.RenderTargetPtr->Info;

      VkAttachmentDescription& attachmentDesc = AttachmentDescs[i];
      attachmentDesc.format  = GetVulkanTextureFormat(RTInfo.Format);
      attachmentDesc.samples = (VkSampleCountFlagBits)RTInfo.SampleCount;
      GetVulkanAttachmentLoadStoreOp(attachmentDesc.loadOp,
                                     attachmentDesc.storeOp,
                                     attachment.LoadStoreOp);
      GetVulkanAttachmentLoadStoreOp(attachmentDesc.stencilLoadOp,
                                     attachmentDesc.stencilStoreOp,
                                     attachment.StencilLoadStoreOp);

      assert((attachment.IsResolveAttachment()
              && ((int32_t)RTInfo.SampleCount > 1))
          || (!attachment.IsResolveAttachment()
              && (int32_t)RTInfo.SampleCount == 1));
      const bool IsInvalidSampleCountAndLayerCount
          = (SampleCount == 0) || (LayerCount == 0);
      if (!attachment.IsDepthAttachment()
          || IsInvalidSampleCountAndLayerCount) {
        assert(SampleCount == 0 || SampleCount == (int32_t)RTInfo.SampleCount);
        assert(LayerCount == 0 || LayerCount == RTInfo.LayerCount);
        SampleCount = (int32_t)RTInfo.SampleCount;
        LayerCount  = RTInfo.LayerCount;
      }
      attachmentDesc.initialLayout
          = GetVulkanImageLayout(attachment.InitialLayout);
      attachmentDesc.finalLayout = GetVulkanImageLayout(attachment.FinalLayout);

      const auto&  RTClearColor = attachment.m_rtClearValue.GetCleraColor();
      VkClearValue clearValue   = {};
      if (attachment.IsDepthAttachment()) {
        clearValue.depthStencil = {attachment.m_rtClearValue.GetCleraDepth(),
                                   attachment.m_rtClearValue.GetCleraStencil()};
      } else {
        clearValue.color = {
          RTClearColor[0], RTClearColor[1], RTClearColor[2], RTClearColor[3]};
      }
      ClearValues.push_back(clearValue);
    }

    struct SubpassAttachmentRefs {
      std::vector<VkAttachmentReference>   InputAttachmentRefs;
      std::vector<VkAttachmentReference>   OutputColorAttachmentRefs;
      std::optional<VkAttachmentReference> OutputDepthAttachmentRef;
      std::optional<VkAttachmentReference> OutputResolveAttachmentRef;
    };

    assert(m_renderPassInfo.Subpasses.size());

    std::vector<SubpassAttachmentRefs> subpassAttachmentRefs;
    subpassAttachmentRefs.resize(m_renderPassInfo.Subpasses.size());

    std::vector<VkSubpassDescription> SubpassDescs;
    SubpassDescs.resize(m_renderPassInfo.Subpasses.size());

    std::vector<VkSubpassDependency> SubpassDependencies;
    SubpassDependencies.resize(m_renderPassInfo.Subpasses.size() + 1);

    const bool IsSubpassForExecuteInOrder
        = m_renderPassInfo.IsSubpassForExecuteInOrder();

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

    for (int32_t i = 0; i < (int32_t)m_renderPassInfo.Subpasses.size(); ++i) {
      const Subpass& subPass = m_renderPassInfo.Subpasses[i];

      std::vector<VkAttachmentReference>& InputAttachmentRefs
          = subpassAttachmentRefs[i].InputAttachmentRefs;
      std::vector<VkAttachmentReference>& OutputColorAttachmentRefs
          = subpassAttachmentRefs[i].OutputColorAttachmentRefs;
      std::optional<VkAttachmentReference>& OutputDepthAttachmentRef
          = subpassAttachmentRefs[i].OutputDepthAttachmentRef;
      std::optional<VkAttachmentReference>& OutputResolveAttachmentRef
          = subpassAttachmentRefs[i].OutputResolveAttachmentRef;

      for (const int32_t& attachmentIndex : subPass.InputAttachments) {
        const VkAttachmentReference attchmentRef = {
          (uint32_t)attachmentIndex, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
        InputAttachmentRefs.emplace_back(attchmentRef);
      }
      for (const int32_t& attachmentIndex : subPass.OutputColorAttachments) {
        const VkAttachmentReference attchmentRef = {
          (uint32_t)attachmentIndex, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
        OutputColorAttachmentRefs.emplace_back(attchmentRef);
      }
      if (subPass.OutputDepthAttachment) {
        const VkAttachmentReference attchmentRef
            = {(uint32_t)subPass.OutputDepthAttachment.value(),
               VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
        OutputDepthAttachmentRef = attchmentRef;
      }
      if (subPass.OutputResolveAttachment) {
        const VkAttachmentReference attchmentRef
            = {(uint32_t)subPass.OutputDepthAttachment.value(),
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
            = (uint32_t)subPass.SourceSubpassIndex;
        SubpassDependencies[DependencyIndex].dstSubpass
            = (uint32_t)subPass.DestSubpassIndex;
      }
      SubpassDependencies[DependencyIndex].srcStageMask
          = GetPipelineStageMask(subPass.AttachmentProducePipelineBit);
      SubpassDependencies[DependencyIndex].dstStageMask
          = GetPipelineStageMask(subPass.AttachmentConsumePipelineBit);
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
            g_rhi_vk->m_device_, &renderPassCreateInfo, nullptr, &m_renderPass)
        != VK_SUCCESS) {
      return false;
    }
  }

  // Create RenderPassBeginInfo
  RenderPassBeginInfo            = {};
  RenderPassBeginInfo.sType      = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  RenderPassBeginInfo.renderPass = m_renderPass;

  RenderPassBeginInfo.renderArea.offset = {RenderOffset.x(), RenderOffset.y()};
  RenderPassBeginInfo.renderArea.extent
      = {(uint32_t)RenderExtent.x(), (uint32_t)RenderExtent.y()};

  RenderPassBeginInfo.clearValueCount
      = static_cast<uint32_t>(ClearValues.size());
  RenderPassBeginInfo.pClearValues = ClearValues.data();

  // Create framebuffer
  {
    std::vector<VkImageView> ImageViews;

    for (int32_t k = 0; k < m_renderPassInfo.Attachments.size(); ++k) {
      assert(m_renderPassInfo.Attachments[k].IsValid());

      const auto* RT = m_renderPassInfo.Attachments[k].RenderTargetPtr.get();
      assert(RT);

      const TextureVk* texture_vk = (const TextureVk*)RT->GetTexture();
      assert(texture_vk);

      ImageViews.push_back(texture_vk->imageView);
    }

    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType      = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = m_renderPass;

    framebufferInfo.attachmentCount = static_cast<uint32_t>(ImageViews.size());
    framebufferInfo.pAttachments    = ImageViews.data();

    framebufferInfo.width  = RenderExtent.x();
    framebufferInfo.height = RenderExtent.y();
    framebufferInfo.layers = LayerCount;

    if (vkCreateFramebuffer(
            g_rhi_vk->m_device_, &framebufferInfo, nullptr, &m_frameBuffer)
        != VK_SUCCESS) {
      return false;
    }
  }

  return true;
}

}  // namespace game_engine
