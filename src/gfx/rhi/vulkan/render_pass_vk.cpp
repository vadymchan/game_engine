
#include "gfx/rhi/vulkan/render_pass_vk.h"

#include "gfx/rhi/vulkan/rhi_vk.h"

namespace game_engine {

// SubpassVk
// =======================================================================

void SubpassVk::Initialize(int32_t            InSourceSubpassIndex,
                           int32_t            InDestSubpassIndex,
                           EPipelineStageMask InAttachmentProducePipelineBit,
                           EPipelineStageMask InAttachmentConsumePipelineBit) {
  SourceSubpassIndex           = InSourceSubpassIndex;
  DestSubpassIndex             = InDestSubpassIndex;
  AttachmentProducePipelineBit = InAttachmentProducePipelineBit;
  AttachmentConsumePipelineBit = InAttachmentConsumePipelineBit;
}

bool SubpassVk::IsSubpassForExecuteInOrder() const {
  if ((SourceSubpassIndex == -1) && (DestSubpassIndex == -1)) {
    return true;
  }

  if (SourceSubpassIndex != -1 && DestSubpassIndex != -1) {
    return false;
  }

  // Subpass indices have to be either -1 for all or not for all.
  // This is an error condition and should be handled appropriately.
  return false;
}

// RenderPassInfoVk
// =======================================================================

// If both SourceSubpass and DstSubpass of all subpasses are -1, subpasses
// will be executed in order
bool RenderPassInfoVk::IsSubpassForExecuteInOrder() const {
  assert(Subpasses.size());

  int32_t i                       = 0;
  bool isSubpassForExecuteInOrder = Subpasses[i++].IsSubpassForExecuteInOrder();
  for (; i < (int32_t)Subpasses.size(); ++i) {
    // All isSubpassForExecuteInOrder of subpasses must be same.
    assert(isSubpassForExecuteInOrder
           == Subpasses[i].IsSubpassForExecuteInOrder());

    if (isSubpassForExecuteInOrder
        != Subpasses[i].IsSubpassForExecuteInOrder()) {
      return false;
    }
  }
  return isSubpassForExecuteInOrder;
}

bool RenderPassInfoVk::Validate() const {
  for (const auto& iter : Subpasses) {
    for (const auto& inputIndex : iter.InputAttachments) {
      if (!Attachments.size() > inputIndex) {
        return false;
      }
    }
    for (const auto& outputIndex : iter.OutputColorAttachments) {
      if (!Attachments.size() > outputIndex) {
        return false;
      }
    }
    if (iter.OutputDepthAttachment) {
      if (!Attachments.size() > iter.OutputDepthAttachment.value()) {
        return false;
      }
    }
    if (iter.OutputResolveAttachment) {
      if (!Attachments.size() > iter.OutputResolveAttachment.value()) {
        return false;
      }
    }
  }
  return true;
}

// RenderPassVk
// =======================================================================

void RenderPassVk::Release() {
  if (FrameBuffer != VK_NULL_HANDLE) {
    vkDestroyFramebuffer(g_rhi_vk->m_device_, FrameBuffer, nullptr);
    FrameBuffer = VK_NULL_HANDLE;
  }
  if (RenderPass != VK_NULL_HANDLE) {
    vkDestroyRenderPass(g_rhi_vk->m_device_, RenderPass, nullptr);
    FrameBuffer = VK_NULL_HANDLE;
  }
}

void RenderPassVk::SetAttachment(
    const std::vector<AttachmentVk>&   colorAttachments,
    const std::optional<AttachmentVk>& depthAttachment        ,
    const std::optional<AttachmentVk>& colorResolveAttachment ) {
  // Add output color attachments
  int32_t startIndex = static_cast<int32_t>(RenderPassInfo.Attachments.size());
  RenderPassInfo.Attachments.insert(RenderPassInfo.Attachments.end(),
                                    colorAttachments.begin(),
                                    colorAttachments.end());

  // Ensure at least one subpass exists
  if (RenderPassInfo.Subpasses.empty()) {
    RenderPassInfo.Subpasses.resize(1);
  }

  // Add indices for output color attachments
  for (int32_t i = 0; i < static_cast<int32_t>(colorAttachments.size()); ++i) {
    RenderPassInfo.Subpasses[0].OutputColorAttachments.push_back(startIndex
                                                                 + i);
  }

  // Add output depth attachment if provided
  if (depthAttachment) {
    startIndex = static_cast<int32_t>(RenderPassInfo.Attachments.size());
    RenderPassInfo.Attachments.push_back(*depthAttachment);
    RenderPassInfo.Subpasses[0].OutputDepthAttachment = startIndex;
  }

  // Add output resolve attachment if provided
  if (colorResolveAttachment) {
    startIndex = static_cast<int32_t>(RenderPassInfo.Attachments.size());
    RenderPassInfo.Attachments.push_back(*colorResolveAttachment);
    RenderPassInfo.Subpasses[0].OutputResolveAttachment = startIndex;
  }
}

void RenderPassVk::SetRenderArea(const math::Vector2Di& offset,
                                 const math::Vector2Di& extent) {
  RenderOffset = offset;
  RenderExtent = extent;
}

void RenderPassVk::SetFinalLayoutToAttachment(
    const AttachmentVk& attachment) const {
  assert(attachment.RenderTargetPtr);
  TextureVk* texture_vk = (TextureVk*)attachment.RenderTargetPtr->GetTexture();
  texture_vk->imageLayout = attachment.FinalLayout;
}

bool RenderPassVk::BeginRenderPass(const CommandBufferVk* commandBuffer,
                                   VkSubpassContents      subpassContents
                                   ) {
  assert(commandBuffer);

  CommandBuffer = commandBuffer;

  assert(FrameBuffer);
  RenderPassBeginInfo.framebuffer = FrameBuffer;

  vkCmdBeginRenderPass((VkCommandBuffer)commandBuffer->GetNativeHandle(),
                       &RenderPassBeginInfo,
                       subpassContents);
  return true;
}

void RenderPassVk::EndRenderPass() {
  assert(CommandBuffer);

  // Finishing up
  vkCmdEndRenderPass((VkCommandBuffer)CommandBuffer->GetNativeHandle());

  // Apply layout to attachments
  for (AttachmentVk& iter : RenderPassInfo.Attachments) {
    assert(iter.IsValid());
    SetFinalLayoutToAttachment(iter);
  }

  CommandBuffer = nullptr;
}

bool RenderPassVk::CreateRenderPass() {
  //  std::vector<VkAttachmentDescription> attachmentDescs;
  //  attachmentDescs.reserve(RenderPassInfo.Attachments.size());

  //  for (const auto& attachment : RenderPassInfo.Attachments) {
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

  // Create RenderPass
  {
    std::vector<VkAttachmentDescription> AttachmentDescs;
    AttachmentDescs.resize(RenderPassInfo.Attachments.size());
    for (int32_t i = 0; i < (int32_t)RenderPassInfo.Attachments.size(); ++i) {
      const AttachmentVk& attachment = RenderPassInfo.Attachments[i];
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

      assert(
          (attachment.IsResolveAttachment && ((int32_t)RTInfo.SampleCount > 1))
          || (!attachment.IsResolveAttachment
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

      const auto&  RTClearColor = attachment.RTClearValue.GetCleraColor();
      VkClearValue clearValue   = {};
      if (attachment.IsDepthAttachment()) {
        clearValue.depthStencil = {attachment.RTClearValue.GetCleraDepth(),
                                   attachment.RTClearValue.GetCleraStencil()};
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

    assert(RenderPassInfo.Subpasses.size());

    std::vector<SubpassAttachmentRefs> subpassAttachmentRefs;
    subpassAttachmentRefs.resize(RenderPassInfo.Subpasses.size());

    std::vector<VkSubpassDescription> SubpassDescs;
    SubpassDescs.resize(RenderPassInfo.Subpasses.size());

    std::vector<VkSubpassDependency> SubpassDependencies;
    SubpassDependencies.resize(RenderPassInfo.Subpasses.size() + 1);

    const bool IsSubpassForExecuteInOrder
        = RenderPassInfo.IsSubpassForExecuteInOrder();

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

    for (int32_t i = 0; i < (int32_t)RenderPassInfo.Subpasses.size(); ++i) {
      const SubpassVk& subPass = RenderPassInfo.Subpasses[i];

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
            g_rhi_vk->m_device_, &renderPassCreateInfo, nullptr, &RenderPass)
        != VK_SUCCESS) {
      return false;
    }
  }

  // Create RenderPassBeginInfo
  RenderPassBeginInfo            = {};
  RenderPassBeginInfo.sType      = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  RenderPassBeginInfo.renderPass = RenderPass;

  RenderPassBeginInfo.renderArea.offset = {RenderOffset.x(), RenderOffset.y()};
  RenderPassBeginInfo.renderArea.extent
      = {(uint32_t)RenderExtent.x(), (uint32_t)RenderExtent.y()};

  RenderPassBeginInfo.clearValueCount
      = static_cast<uint32_t>(ClearValues.size());
  RenderPassBeginInfo.pClearValues = ClearValues.data();

  // Create framebuffer
  {
    std::vector<VkImageView> ImageViews;

    for (int32_t k = 0; k < RenderPassInfo.Attachments.size(); ++k) {
      assert(RenderPassInfo.Attachments[k].IsValid());

      const auto* RT = RenderPassInfo.Attachments[k].RenderTargetPtr.get();
      assert(RT);

      const TextureVk* texture_vk = (const TextureVk*)RT->GetTexture();
      assert(texture_vk);

      ImageViews.push_back(texture_vk->imageView);
    }

    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType      = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = RenderPass;

    framebufferInfo.attachmentCount = static_cast<uint32_t>(ImageViews.size());
    framebufferInfo.pAttachments    = ImageViews.data();

    framebufferInfo.width  = RenderExtent.x();
    framebufferInfo.height = RenderExtent.y();
    framebufferInfo.layers = LayerCount;

    if (vkCreateFramebuffer(
            g_rhi_vk->m_device_, &framebufferInfo, nullptr, &FrameBuffer)
        != VK_SUCCESS) {
      return false;
    }
  }

  return true;
}

}  // namespace game_engine
