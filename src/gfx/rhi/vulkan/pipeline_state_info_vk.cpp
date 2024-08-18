
#include "gfx/rhi/vulkan/pipeline_state_info_vk.h"

#include "gfx/rhi/vulkan/rhi_vk.h"
#include "utils/third_party/xxhash_util.h"

namespace game_engine {

// SamplerStateInfoVk
// ======================================================================

void SamplerStateInfoVk::Initialize() {
  GetHash();
  m_samplerStateInfo_.sType     = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  m_samplerStateInfo_.magFilter = GetVulkanTextureFilterType(m_magnification_);
  m_samplerStateInfo_.minFilter = GetVulkanTextureFilterType(m_minification_);

  m_samplerStateInfo_.addressModeU = GetVulkanTextureAddressMode(m_addressU_);
  m_samplerStateInfo_.addressModeV = GetVulkanTextureAddressMode(m_addressV_);
  m_samplerStateInfo_.addressModeW = GetVulkanTextureAddressMode(m_addressW_);

  m_samplerStateInfo_.anisotropyEnable = (m_maxAnisotropy_ > 1);
  m_samplerStateInfo_.maxAnisotropy    = m_maxAnisotropy_;

  m_samplerStateInfo_.unnormalizedCoordinates = VK_FALSE;

  m_samplerStateInfo_.compareEnable = m_isEnableComparisonMode_;
  m_samplerStateInfo_.compareOp     = GetVulkanCompareOp(m_comparisonFunc_);

  m_samplerStateInfo_.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  m_samplerStateInfo_.mipLodBias = 0.0f;    // Optional
  m_samplerStateInfo_.minLod     = m_minLOD_;  // Optional
  m_samplerStateInfo_.maxLod     = m_maxLOD_;

  m_samplerStateInfo_.borderColor = VK_BORDER_COLOR_FLOAT_CUSTOM_EXT;

  VkSamplerCustomBorderColorCreateInfoEXT CustomBorderColor{};
  CustomBorderColor.sType
      = VK_STRUCTURE_TYPE_SAMPLER_CUSTOM_BORDER_COLOR_CREATE_INFO_EXT;
  memcpy(CustomBorderColor.customBorderColor.float32,
         &m_borderColor_,
         sizeof(m_borderColor_));
  m_samplerStateInfo_.pNext = &CustomBorderColor;

  if (vkCreateSampler(
          g_rhi_vk->m_device_, &m_samplerStateInfo_, nullptr, &m_samplerState_)
      != VK_SUCCESS) {
    GlobalLogger::Log(LogLevel::Error, "Failed to create sampler");
  }

  m_resourceName_ = Name(ToString().c_str());
}

void SamplerStateInfoVk::Release() {
  if (m_samplerState_) {
    vkDestroySampler(g_rhi_vk->m_device_, m_samplerState_, nullptr);
    m_samplerState_ = nullptr;
  }
}

// RasterizationStateInfoVk
// ======================================================================

void RasterizationStateInfoVk::Initialize() {
  GetHash();
  m_rasterizationStateInfo_ = {};
  m_rasterizationStateInfo_.sType
      = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  m_rasterizationStateInfo_.depthClampEnable        = m_depthClampEnable_;
  m_rasterizationStateInfo_.rasterizerDiscardEnable = m_rasterizerDiscardEnable_;
  m_rasterizationStateInfo_.polygonMode
      = GetVulkanPolygonMode(m_polygonMode_);  // FILL, LINE, POINT
  m_rasterizationStateInfo_.lineWidth       = m_lineWidth_;
  m_rasterizationStateInfo_.cullMode        = GetVulkanCullMode(m_cullMode_);
  m_rasterizationStateInfo_.frontFace       = GetVulkanFrontFace(m_frontFace_);
  m_rasterizationStateInfo_.depthBiasEnable = m_depthBiasEnable_;
  m_rasterizationStateInfo_.depthBiasConstantFactor
      = m_depthBiasConstantFactor_;                           // Optional
  m_rasterizationStateInfo_.depthBiasClamp = m_depthBiasClamp_;  // Optional
  m_rasterizationStateInfo_.depthBiasSlopeFactor
      = m_depthBiasSlopeFactor_;                              // Optional

  // VkPipelineRasterizationStateCreateFlags flags;

  m_multisampleStateInfo_ = {};
  m_multisampleStateInfo_.sType
      = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  m_multisampleStateInfo_.rasterizationSamples
      = (VkSampleCountFlagBits)m_sampleCount_;
#if USE_VARIABLE_SHADING_RATE_TIER2
  MultisampleStateInfo.sampleShadingEnable = SampleShadingEnable;
#else
  m_multisampleStateInfo_.sampleShadingEnable = false;
#endif
  m_multisampleStateInfo_.minSampleShading = m_minSampleShading_;
  m_multisampleStateInfo_.alphaToCoverageEnable
      = m_alphaToCoverageEnable_;                               // Optional
  m_multisampleStateInfo_.alphaToOneEnable = m_alphaToOneEnable_;  // Optional
}

// StencilOpStateInfoVk
// ======================================================================

void StencilOpStateInfoVk::Initialize() {
  GetHash();
  m_stencilOpStateInfo_             = {};
  m_stencilOpStateInfo_.failOp      = GetVulkanStencilOp(m_failOp_);
  m_stencilOpStateInfo_.passOp      = GetVulkanStencilOp(m_passOp_);
  m_stencilOpStateInfo_.depthFailOp = GetVulkanStencilOp(m_depthFailOp_);
  m_stencilOpStateInfo_.compareOp   = GetVulkanCompareOp(m_compareOp_);
  m_stencilOpStateInfo_.compareMask = m_compareMask_;
  m_stencilOpStateInfo_.writeMask   = m_writeMask_;
  m_stencilOpStateInfo_.reference   = m_reference_;
}

// DepthStencilStateInfoVk
// ======================================================================

void DepthStencilStateInfoVk::Initialize() {
  GetHash();
  m_depthStencilStateInfo_ = {};
  m_depthStencilStateInfo_.sType
      = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  m_depthStencilStateInfo_.depthTestEnable  = m_depthTestEnable_;
  m_depthStencilStateInfo_.depthWriteEnable = m_depthWriteEnable_;
  m_depthStencilStateInfo_.depthCompareOp   = GetVulkanCompareOp(m_depthCompareOp_);
  m_depthStencilStateInfo_.depthBoundsTestEnable = m_depthBoundsTestEnable_;
  m_depthStencilStateInfo_.minDepthBounds        = m_minDepthBounds_;  // Optional
  m_depthStencilStateInfo_.maxDepthBounds        = m_maxDepthBounds_;  // Optional
  m_depthStencilStateInfo_.stencilTestEnable     = m_stencilTestEnable_;
  if (m_front_) {
    m_depthStencilStateInfo_.front
        = ((StencilOpStateInfoVk*)m_front_)->m_stencilOpStateInfo_;
  }
  if (m_back_) {
    m_depthStencilStateInfo_.back
        = ((StencilOpStateInfoVk*)m_back_)->m_stencilOpStateInfo_;
  }
}

// BlendingStateInfoVk
// ======================================================================

void BlendingStateInfoVk::Initialize() {
  GetHash();
  m_colorBlendAttachmentInfo_                     = {};
  m_colorBlendAttachmentInfo_.blendEnable         = m_blendEnable_;
  m_colorBlendAttachmentInfo_.srcColorBlendFactor = GetVulkanBlendFactor(m_src_);
  m_colorBlendAttachmentInfo_.dstColorBlendFactor = GetVulkanBlendFactor(m_dest_);
  m_colorBlendAttachmentInfo_.colorBlendOp        = GetVulkanBlendOp(m_blendOp_);
  m_colorBlendAttachmentInfo_.srcAlphaBlendFactor = GetVulkanBlendFactor(m_srcAlpha_);
  m_colorBlendAttachmentInfo_.dstAlphaBlendFactor
      = GetVulkanBlendFactor(m_destAlpha_);
  m_colorBlendAttachmentInfo_.alphaBlendOp   = GetVulkanBlendOp(m_alphaBlendOp_);
  m_colorBlendAttachmentInfo_.colorWriteMask = GetVulkanColorMask(m_colorWriteMask_);
}

// PipelineStateFixedInfoVk
// ======================================================================

//// PushConstantVk
//// ======================================================================
//
// PushConstantVk::PushConstantVk(const PushConstantVk& pushConstant) {
//  assert(pushConstant.UsedSize < 256);
//
//  UsedSize = pushConstant.UsedSize;
//  memcpy(Data, pushConstant.Data, pushConstant.UsedSize);
//  PushConstantRanges = pushConstant.PushConstantRanges;
//  Hash               = pushConstant.Hash;
//}
//
// PushConstantVk::PushConstantVk(const char*            data,
//                               int32_t                size,
//                               EShaderAccessStageFlag shaderAccessStageFlag)
//                               {
//  assert(size < 256);
//
//  UsedSize = size;
//  memcpy(Data, data, size);
//  PushConstantRanges.Add(
//      PushConstantRangeVk(shaderAccessStageFlag, 0, size));
//  GetHash();
//}
//
// PushConstantVk::PushConstantVk(const char*                data,
//                               int32_t                    size,
//                               const PushConstantRangeVk& pushConstantRange)
//                               {
//  assert(size < 256);
//
//  UsedSize = size;
//  memcpy(Data, data, size);
//  PushConstantRanges.Add(pushConstantRange);
//  GetHash();
//}
//
// PushConstantVk::PushConstantVk(
//    const char*                                   data,
//    int32_t                                       size,
//    const ResourceContainer<PushConstantRangeVk>& pushConstantRanges)
//    : PushConstantRanges(pushConstantRanges) {
//  assert(size < 256);
//
//  UsedSize = size;
//  memcpy(Data, data, size);
//  GetHash();
//}
//
// PushConstantVk& PushConstantVk::operator=(
//    const PushConstantVk& pushConstant) {
//  assert(pushConstant.UsedSize < 256);
//
//  UsedSize = pushConstant.UsedSize;
//  memcpy(Data, pushConstant.Data, pushConstant.UsedSize);
//  PushConstantRanges = pushConstant.PushConstantRanges;
//  Hash               = pushConstant.Hash;
//  return *this;
//}
//
// size_t PushConstantVk::GetHash() const {
//  if (Hash) {
//    return Hash;
//  }
//
//  Hash = 0;
//  if (PushConstantRanges.NumOfData > 0) {
//    Hash = ::XXH64(&PushConstantRanges[0],
//                   sizeof(PushConstantRangeVk) * PushConstantRanges.NumOfData,
//                   Hash);
//  }
//
//  return Hash;
//}

// PipelineStateInfoVk
// ======================================================================

void PipelineStateInfoVk::Initialize() {
  GetHash();
  if (m_pipelineType_ == EPipelineType::Graphics) {
    CreateGraphicsPipelineState();
  } else if (m_pipelineType_ == EPipelineType::Compute) {
    CreateComputePipelineState();
  } else if (m_pipelineType_ == EPipelineType::RayTracing) {
    // TODO:
    // CreateRaytracingPipelineState();
    assert(0);
  } else {
    assert(0);
  }
}

void PipelineStateInfoVk::Release() {
  if (m_pipeline_) {
    vkDestroyPipeline(g_rhi_vk->m_device_, m_pipeline_, nullptr);
    m_pipeline_ = nullptr;
  }
  m_pipelineLayout_ = nullptr;
}

void* PipelineStateInfoVk::CreateGraphicsPipelineState() {
  if (m_pipeline_) {
    return m_pipeline_;
  }

  assert(m_pipelineStateFixed_);
  assert(m_vertexBufferArray.m_numOfData_);

  VkPipelineVertexInputStateCreateInfo vertexInputInfo2
      = ((VertexBufferVk*)m_vertexBufferArray[0])->CreateVertexInputState();

  std::vector<VkVertexInputBindingDescription>   bindingDescriptions;
  std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

  VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
  VertexBufferVk::CreateVertexInputState(vertexInputInfo,
                                         bindingDescriptions,
                                         attributeDescriptions,
                                         m_vertexBufferArray);

  VkPipelineInputAssemblyStateCreateInfo inputAssembly
      = ((VertexBufferVk*)m_vertexBufferArray[0])->CreateInputAssemblyState();

  const auto&             Viewports = m_pipelineStateFixed_->m_viewports_;
  std::vector<VkViewport> vkViewports;
  vkViewports.resize(Viewports.size());
  for (int32_t i = 0; i < vkViewports.size(); ++i) {
    vkViewports[i].x     = Viewports[i].m_x_;
    vkViewports[i].width = Viewports[i].m_width_;

    if (kUseVulkanNdcYFlip) {
      // vkViewports[i].y      = Viewports[i].Height - Viewports[i].Y; //
      // previous soulution
      vkViewports[i].y
          = Viewports[i].m_y_ + Viewports[i].m_height_;  // another solution
      // vkViewports[i].y      = Viewports[i].Height;
      vkViewports[i].height = -Viewports[i].m_height_;
    } else {
      vkViewports[i].y      = Viewports[i].m_y_;
      vkViewports[i].height = Viewports[i].m_height_;
    }

    vkViewports[i].minDepth = Viewports[i].m_minDepth_;
    vkViewports[i].maxDepth = Viewports[i].m_maxDepth_;
  }

  const auto&           Scissors = m_pipelineStateFixed_->m_scissors_;
  std::vector<VkRect2D> vkScissor;
  vkScissor.resize(Scissors.size());
  for (int32_t i = 0; i < vkScissor.size(); ++i) {
    vkScissor[i].offset.x      = Scissors[i].m_offset_.x();
    vkScissor[i].offset.y      = Scissors[i].m_offset_.y();
    vkScissor[i].extent.width  = Scissors[i].m_extent_.width();
    vkScissor[i].extent.height = Scissors[i].m_extent_.height();
  }

  VkPipelineViewportStateCreateInfo viewportState = {};
  viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportState.viewportCount
      = std::max(1u, static_cast<uint32_t>(vkViewports.size()));
  viewportState.pViewports = vkViewports.size() ? &vkViewports[0] : nullptr;
  viewportState.scissorCount
      = std::max(1u, static_cast<uint32_t>(vkScissor.size()));
  viewportState.pScissors = vkScissor.size() ? &vkScissor[0] : nullptr;

  VkPipelineColorBlendStateCreateInfo colorBlending = {};
  colorBlending.sType
      = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorBlending.logicOpEnable = VK_FALSE;
  colorBlending.logicOp       = VK_LOGIC_OP_COPY;  // Optional

  int32_t ColorAttachmentCountInSubpass = 0;
  assert(m_renderPass->m_renderPassInfo_.m_subpasses_.size() > m_subpassIndex_);
  const Subpass& SelectedSubpass
      = m_renderPass->m_renderPassInfo_.m_subpasses_[m_subpassIndex_];
  for (int32_t i = 0;
       i < (int32_t)SelectedSubpass.m_outputColorAttachments_.size();
       ++i) {
    const int32_t AttachmentIndex = SelectedSubpass.m_outputColorAttachments_[i];
    const bool    IsColorAttachment
        = !m_renderPass->m_renderPassInfo_.m_attachments_[AttachmentIndex]
               .IsDepthAttachment()
       && !m_renderPass->m_renderPassInfo_.m_attachments_[AttachmentIndex]
               .IsResolveAttachment();
    if (IsColorAttachment) {
      ++ColorAttachmentCountInSubpass;
    }
  }

  std::vector<VkPipelineColorBlendAttachmentState> ColorBlendAttachmentStates;
  if (ColorAttachmentCountInSubpass > 1) {
    ColorBlendAttachmentStates.resize(
        ColorAttachmentCountInSubpass,
        ((BlendingStateInfoVk*)m_pipelineStateFixed_->m_blendingState_)
            ->m_colorBlendAttachmentInfo_);
    colorBlending.attachmentCount = ColorAttachmentCountInSubpass;
    colorBlending.pAttachments    = &ColorBlendAttachmentStates[0];
  } else {
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments
        = &((BlendingStateInfoVk*)m_pipelineStateFixed_->m_blendingState_)
               ->m_colorBlendAttachmentInfo_;
  }

  colorBlending.blendConstants[0] = 0.0f;  // Optional
  colorBlending.blendConstants[1] = 0.0f;  // Optional
  colorBlending.blendConstants[2] = 0.0f;  // Optional
  colorBlending.blendConstants[3] = 0.0f;  // Optional

  // 9. Dynamic state
  VkPipelineDynamicStateCreateInfo dynamicState = {};
  dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;

  std::vector<VkDynamicState> dynamicStates;
  if (m_pipelineStateFixed_->m_dynamicStates_.size() > 0) {
    dynamicStates.resize(m_pipelineStateFixed_->m_dynamicStates_.size());

    for (int32_t i = 0; i < (int32_t)m_pipelineStateFixed_->m_dynamicStates_.size();
         ++i) {
      dynamicStates[i]
          = GetVulkanPipelineDynamicState(m_pipelineStateFixed_->m_dynamicStates_[i]);
    }

    dynamicState.dynamicStateCount = (uint32_t)dynamicStates.size();
    dynamicState.pDynamicStates    = &dynamicStates[0];
  }

  // 10. Pipeline layout
  m_pipelineLayout_ = ShaderBindingLayoutVk::CreatePipelineLayout(
      m_shaderBindingLayoutArray, m_pushConstant);

  VkGraphicsPipelineCreateInfo pipelineInfo = {};
  pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

  // Shader stage
  VkPipelineShaderStageCreateInfo ShaderStages[5];
  uint32_t                        ShaderStageIndex = 0;
  if (m_graphicsShader_.m_vertexShader_) {
    ShaderStages[ShaderStageIndex++]
        = ((CompiledShaderVk*)m_graphicsShader_.m_vertexShader_->GetCompiledShader())
              ->m_shaderStage_;
  }
  if (m_graphicsShader_.m_geometryShader_) {
    ShaderStages[ShaderStageIndex++]
        = ((CompiledShaderVk*)m_graphicsShader_.m_geometryShader_)->m_shaderStage_;
  }
  if (m_graphicsShader_.m_pixelShader_) {
    ShaderStages[ShaderStageIndex++]
        //= ((CompiledShaderVk*)GraphicsShader.m_pixelShader_)->m_shaderStage_;
        = ((CompiledShaderVk*)m_graphicsShader_.m_pixelShader_->GetCompiledShader())
			  ->m_shaderStage_;
  }

  assert(ShaderStageIndex > 0);
  pipelineInfo.stageCount = ShaderStageIndex;
  pipelineInfo.pStages    = &ShaderStages[0];

  // Fixed-function stage
  pipelineInfo.pVertexInputState   = &vertexInputInfo;
  pipelineInfo.pInputAssemblyState = &inputAssembly;
  pipelineInfo.pViewportState      = &viewportState;
  pipelineInfo.pRasterizationState
      = &((RasterizationStateInfoVk*)m_pipelineStateFixed_->m_rasterizationState_)
             ->m_rasterizationStateInfo_;
  pipelineInfo.pMultisampleState
      = &((RasterizationStateInfoVk*)m_pipelineStateFixed_->m_rasterizationState_)
             ->m_multisampleStateInfo_;
  pipelineInfo.pDepthStencilState
      = &((DepthStencilStateInfoVk*)m_pipelineStateFixed_->m_depthStencilState_)
             ->m_depthStencilStateInfo_;
  pipelineInfo.pColorBlendState = &colorBlending;
  pipelineInfo.pDynamicState    = &dynamicState;
  pipelineInfo.layout           = m_pipelineLayout_;
  pipelineInfo.renderPass       = (VkRenderPass)m_renderPass->GetRenderPass();
  pipelineInfo.subpass          = m_subpassIndex_;      // index of subpass

  pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;  // Optional
  pipelineInfo.basePipelineIndex  = -1;              // Optional

  VkShadingRatePaletteNV shadingRatePalette{};
  VkPipelineViewportShadingRateImageStateCreateInfoNV
      pipelineViewportShadingRateImageStateCI{};

  static const std::vector<VkShadingRatePaletteEntryNV>
      shadingRatePaletteEntries = {
        VK_SHADING_RATE_PALETTE_ENTRY_NO_INVOCATIONS_NV,
        VK_SHADING_RATE_PALETTE_ENTRY_16_INVOCATIONS_PER_PIXEL_NV,
        VK_SHADING_RATE_PALETTE_ENTRY_8_INVOCATIONS_PER_PIXEL_NV,
        VK_SHADING_RATE_PALETTE_ENTRY_4_INVOCATIONS_PER_PIXEL_NV,
        VK_SHADING_RATE_PALETTE_ENTRY_2_INVOCATIONS_PER_PIXEL_NV,
        VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_PIXEL_NV,
        VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_2X1_PIXELS_NV,
        VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_1X2_PIXELS_NV,
        VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_2X2_PIXELS_NV,
        VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_4X2_PIXELS_NV,
        VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_2X4_PIXELS_NV,
        VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_4X4_PIXELS_NV,
      };

  if (m_pipelineStateFixed_->m_isUseVRS_) {
    // Shading Rate Palette
    shadingRatePalette.shadingRatePaletteEntryCount
        = static_cast<uint32_t>(shadingRatePaletteEntries.size());
    shadingRatePalette.pShadingRatePaletteEntries
        = shadingRatePaletteEntries.data();

    pipelineViewportShadingRateImageStateCI.sType
        = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_SHADING_RATE_IMAGE_STATE_CREATE_INFO_NV;
    pipelineViewportShadingRateImageStateCI.shadingRateImageEnable = VK_TRUE;
    pipelineViewportShadingRateImageStateCI.viewportCount
        = viewportState.viewportCount;
    pipelineViewportShadingRateImageStateCI.pShadingRatePalettes
        = &shadingRatePalette;
    viewportState.pNext = &pipelineViewportShadingRateImageStateCI;
  }

  if (vkCreateGraphicsPipelines(g_rhi_vk->m_device_,
                                g_rhi_vk->m_pipelineCache_,
                                1,
                                &pipelineInfo,
                                nullptr,
                                &m_pipeline_)
      != VK_SUCCESS) {
    GlobalLogger::Log(LogLevel::Error, "Failed to create graphics pipeline");
    return nullptr;
  }

  size_t hash = GetHash();
  assert(hash);
  // TODO: real time shader update (WIP)

  // if (GraphicsShader.m_vertexShader_) {
  //   Shader::gConnectedPipelineStateHash[GraphicsShader.m_vertexShader_]
  //       .push_back(hash);
  // }
  // if (GraphicsShader.m_geometryShader_) {
  //   Shader::gConnectedPipelineStateHash[GraphicsShader.m_geometryShader_]
  //       .push_back(hash);
  // }
  // if (GraphicsShader.m_pixelShader_) {
  //   Shader::gConnectedPipelineStateHash[GraphicsShader.m_pixelShader_]
  //       .push_back(hash);
  // }

  return m_pipeline_;
}

void* PipelineStateInfoVk::CreateComputePipelineState() {
  if (m_pipeline_) {
    return m_pipeline_;
  }

  m_pipelineLayout_ = ShaderBindingLayoutVk::CreatePipelineLayout(
      m_shaderBindingLayoutArray, m_pushConstant);

  VkComputePipelineCreateInfo computePipelineCreateInfo{};
  computePipelineCreateInfo.sType
      = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
  computePipelineCreateInfo.layout = m_pipelineLayout_;
  computePipelineCreateInfo.flags  = 0;
  computePipelineCreateInfo.stage
      = ((CompiledShaderVk*)m_computeShader_->GetCompiledShader())->m_shaderStage_;

  if (vkCreateComputePipelines(g_rhi_vk->m_device_,
                               g_rhi_vk->m_pipelineCache_,
                               1,
                               &computePipelineCreateInfo,
                               nullptr,
                               &m_pipeline_)
      != VK_SUCCESS) {
    GlobalLogger::Log(LogLevel::Error, "Failed to create compute pipeline");
    return nullptr;
  }

  size_t hash = GetHash();
  assert(hash);

  // TODO: real time shader update (WIP)
  // if (ComputeShader) {
  //  Shader::gConnectedPipelineStateHash[ComputeShader].push_back(hash);
  //}

  return m_pipeline_;
}

void PipelineStateInfoVk::Bind(
    const std::shared_ptr<RenderFrameContext>& renderFrameContext) const {
  assert(m_pipeline_);
  if (m_pipelineType_ == EPipelineType::Graphics) {
    vkCmdBindPipeline(
        (VkCommandBuffer)renderFrameContext->GetActiveCommandBuffer()
            ->GetNativeHandle(),
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        m_pipeline_);
  } else if (m_pipelineType_ == EPipelineType::Compute) {
    vkCmdBindPipeline(
        (VkCommandBuffer)renderFrameContext->GetActiveCommandBuffer()
            ->GetNativeHandle(),
        VK_PIPELINE_BIND_POINT_COMPUTE,
        m_pipeline_);
  } else if (m_pipelineType_ == EPipelineType::RayTracing) {
    // TODO: implement
    assert(0);
  }
}

}  // namespace game_engine
