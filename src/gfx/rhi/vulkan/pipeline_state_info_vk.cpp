
#include "gfx/rhi/vulkan/pipeline_state_info_vk.h"

#include "gfx/rhi/vulkan/rhi_vk.h"
#include "utils/third_party/xxhash_util.h"

namespace game_engine {

// SamplerStateInfoVk
// ======================================================================

void SamplerStateInfoVk::Initialize() {
  GetHash();
  SamplerStateInfo.sType     = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  SamplerStateInfo.magFilter = GetVulkanTextureFilterType(Magnification);
  SamplerStateInfo.minFilter = GetVulkanTextureFilterType(Minification);

  SamplerStateInfo.addressModeU = GetVulkanTextureAddressMode(AddressU);
  SamplerStateInfo.addressModeV = GetVulkanTextureAddressMode(AddressV);
  SamplerStateInfo.addressModeW = GetVulkanTextureAddressMode(AddressW);

  SamplerStateInfo.anisotropyEnable = (MaxAnisotropy > 1);
  SamplerStateInfo.maxAnisotropy    = MaxAnisotropy;

  SamplerStateInfo.unnormalizedCoordinates = VK_FALSE;

  SamplerStateInfo.compareEnable = IsEnableComparisonMode;
  SamplerStateInfo.compareOp     = GetVulkanCompareOp(ComparisonFunc);

  SamplerStateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  SamplerStateInfo.mipLodBias = 0.0f;    // Optional
  SamplerStateInfo.minLod     = MinLOD;  // Optional
  SamplerStateInfo.maxLod     = MaxLOD;

  SamplerStateInfo.borderColor = VK_BORDER_COLOR_FLOAT_CUSTOM_EXT;

  VkSamplerCustomBorderColorCreateInfoEXT CustomBorderColor{};
  CustomBorderColor.sType
      = VK_STRUCTURE_TYPE_SAMPLER_CUSTOM_BORDER_COLOR_CREATE_INFO_EXT;
  memcpy(CustomBorderColor.customBorderColor.float32,
         &BorderColor,
         sizeof(BorderColor));
  SamplerStateInfo.pNext = &CustomBorderColor;

  if (vkCreateSampler(
          g_rhi_vk->m_device_, &SamplerStateInfo, nullptr, &SamplerState)
      != VK_SUCCESS) {
    GlobalLogger::Log(LogLevel::Error, "Failed to create sampler");
  }

  ResourceName = Name(ToString().c_str());
}

void SamplerStateInfoVk::Release() {
  if (SamplerState) {
    vkDestroySampler(g_rhi_vk->m_device_, SamplerState, nullptr);
    SamplerState = nullptr;
  }
}

// RasterizationStateInfoVk
// ======================================================================

void RasterizationStateInfoVk::Initialize() {
  GetHash();
  RasterizationStateInfo = {};
  RasterizationStateInfo.sType
      = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  RasterizationStateInfo.depthClampEnable        = DepthClampEnable;
  RasterizationStateInfo.rasterizerDiscardEnable = RasterizerDiscardEnable;
  RasterizationStateInfo.polygonMode
      = GetVulkanPolygonMode(PolygonMode);  // FILL, LINE, POINT
  RasterizationStateInfo.lineWidth       = LineWidth;
  RasterizationStateInfo.cullMode        = GetVulkanCullMode(CullMode);
  RasterizationStateInfo.frontFace       = GetVulkanFrontFace(FrontFace);
  RasterizationStateInfo.depthBiasEnable = DepthBiasEnable;
  RasterizationStateInfo.depthBiasConstantFactor
      = DepthBiasConstantFactor;                           // Optional
  RasterizationStateInfo.depthBiasClamp = DepthBiasClamp;  // Optional
  RasterizationStateInfo.depthBiasSlopeFactor
      = DepthBiasSlopeFactor;                              // Optional

  // VkPipelineRasterizationStateCreateFlags flags;

  MultisampleStateInfo = {};
  MultisampleStateInfo.sType
      = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  MultisampleStateInfo.rasterizationSamples
      = (VkSampleCountFlagBits)SampleCount;
#if USE_VARIABLE_SHADING_RATE_TIER2
  MultisampleStateInfo.sampleShadingEnable = SampleShadingEnable;
#else
  MultisampleStateInfo.sampleShadingEnable = false;
#endif
  MultisampleStateInfo.minSampleShading = MinSampleShading;
  MultisampleStateInfo.alphaToCoverageEnable
      = AlphaToCoverageEnable;                               // Optional
  MultisampleStateInfo.alphaToOneEnable = AlphaToOneEnable;  // Optional
}

// StencilOpStateInfoVk
// ======================================================================

void StencilOpStateInfoVk::Initialize() {
  GetHash();
  StencilOpStateInfo             = {};
  StencilOpStateInfo.failOp      = GetVulkanStencilOp(FailOp);
  StencilOpStateInfo.passOp      = GetVulkanStencilOp(PassOp);
  StencilOpStateInfo.depthFailOp = GetVulkanStencilOp(DepthFailOp);
  StencilOpStateInfo.compareOp   = GetVulkanCompareOp(CompareOp);
  StencilOpStateInfo.compareMask = CompareMask;
  StencilOpStateInfo.writeMask   = WriteMask;
  StencilOpStateInfo.reference   = Reference;
}

// DepthStencilStateInfoVk
// ======================================================================

void DepthStencilStateInfoVk::Initialize() {
  GetHash();
  DepthStencilStateInfo = {};
  DepthStencilStateInfo.sType
      = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  DepthStencilStateInfo.depthTestEnable  = DepthTestEnable;
  DepthStencilStateInfo.depthWriteEnable = DepthWriteEnable;
  DepthStencilStateInfo.depthCompareOp   = GetVulkanCompareOp(DepthCompareOp);
  DepthStencilStateInfo.depthBoundsTestEnable = DepthBoundsTestEnable;
  DepthStencilStateInfo.minDepthBounds        = MinDepthBounds;  // Optional
  DepthStencilStateInfo.maxDepthBounds        = MaxDepthBounds;  // Optional
  DepthStencilStateInfo.stencilTestEnable     = StencilTestEnable;
  if (Front) {
    DepthStencilStateInfo.front
        = ((StencilOpStateInfoVk*)Front)->StencilOpStateInfo;
  }
  if (Back) {
    DepthStencilStateInfo.back
        = ((StencilOpStateInfoVk*)Back)->StencilOpStateInfo;
  }
}

// BlendingStateInfoVk
// ======================================================================

void BlendingStateInfoVk::Initialize() {
  GetHash();
  ColorBlendAttachmentInfo                     = {};
  ColorBlendAttachmentInfo.blendEnable         = BlendEnable;
  ColorBlendAttachmentInfo.srcColorBlendFactor = GetVulkanBlendFactor(Src);
  ColorBlendAttachmentInfo.dstColorBlendFactor = GetVulkanBlendFactor(Dest);
  ColorBlendAttachmentInfo.colorBlendOp        = GetVulkanBlendOp(BlendOp);
  ColorBlendAttachmentInfo.srcAlphaBlendFactor = GetVulkanBlendFactor(SrcAlpha);
  ColorBlendAttachmentInfo.dstAlphaBlendFactor
      = GetVulkanBlendFactor(DestAlpha);
  ColorBlendAttachmentInfo.alphaBlendOp   = GetVulkanBlendOp(AlphaBlendOp);
  ColorBlendAttachmentInfo.colorWriteMask = GetVulkanColorMask(ColorWriteMask);
}

// PipelineStateFixedInfoVk
// ======================================================================

//// PushConstantVk
//// ======================================================================
//
// PushConstantVk::PushConstantVk(const PushConstantVk& InPushConstant) {
//  assert(InPushConstant.UsedSize < 256);
//
//  UsedSize = InPushConstant.UsedSize;
//  memcpy(Data, InPushConstant.Data, InPushConstant.UsedSize);
//  PushConstantRanges = InPushConstant.PushConstantRanges;
//  Hash               = InPushConstant.Hash;
//}
//
// PushConstantVk::PushConstantVk(const char*            InData,
//                               int32_t                InSize,
//                               EShaderAccessStageFlag InShaderAccessStageFlag)
//                               {
//  assert(InSize < 256);
//
//  UsedSize = InSize;
//  memcpy(Data, InData, InSize);
//  PushConstantRanges.Add(
//      PushConstantRangeVk(InShaderAccessStageFlag, 0, InSize));
//  GetHash();
//}
//
// PushConstantVk::PushConstantVk(const char*                InData,
//                               int32_t                    InSize,
//                               const PushConstantRangeVk& InPushConstantRange)
//                               {
//  assert(InSize < 256);
//
//  UsedSize = InSize;
//  memcpy(Data, InData, InSize);
//  PushConstantRanges.Add(InPushConstantRange);
//  GetHash();
//}
//
// PushConstantVk::PushConstantVk(
//    const char*                                   InData,
//    int32_t                                       InSize,
//    const ResourceContainer<PushConstantRangeVk>& InPushConstantRanges)
//    : PushConstantRanges(InPushConstantRanges) {
//  assert(InSize < 256);
//
//  UsedSize = InSize;
//  memcpy(Data, InData, InSize);
//  GetHash();
//}
//
// PushConstantVk& PushConstantVk::operator=(
//    const PushConstantVk& InPushConstant) {
//  assert(InPushConstant.UsedSize < 256);
//
//  UsedSize = InPushConstant.UsedSize;
//  memcpy(Data, InPushConstant.Data, InPushConstant.UsedSize);
//  PushConstantRanges = InPushConstant.PushConstantRanges;
//  Hash               = InPushConstant.Hash;
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
  if (PipelineType == EPipelineType::Graphics) {
    CreateGraphicsPipelineState();
  } else if (PipelineType == EPipelineType::Compute) {
    CreateComputePipelineState();
  } else if (PipelineType == EPipelineType::RayTracing) {
    // TODO:
    // CreateRaytracingPipelineState();
    assert(0);
  } else {
    assert(0);
  }
}

void PipelineStateInfoVk::Release() {
  if (vkPipeline) {
    vkDestroyPipeline(g_rhi_vk->m_device_, vkPipeline, nullptr);
    vkPipeline = nullptr;
  }
  vkPipelineLayout = nullptr;
}

void* PipelineStateInfoVk::CreateGraphicsPipelineState() {
  if (vkPipeline) {
    return vkPipeline;
  }

  assert(PipelineStateFixed);
  assert(VertexBufferArray.NumOfData);

  VkPipelineVertexInputStateCreateInfo vertexInputInfo2
      = ((VertexBufferVk*)VertexBufferArray[0])->CreateVertexInputState();

  std::vector<VkVertexInputBindingDescription>   bindingDescriptions;
  std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

  VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
  VertexBufferVk::CreateVertexInputState(vertexInputInfo,
                                         bindingDescriptions,
                                         attributeDescriptions,
                                         VertexBufferArray);

  VkPipelineInputAssemblyStateCreateInfo inputAssembly
      = ((VertexBufferVk*)VertexBufferArray[0])->CreateInputAssemblyState();

  const auto&             Viewports = PipelineStateFixed->viewports;
  std::vector<VkViewport> vkViewports;
  vkViewports.resize(Viewports.size());
  for (int32_t i = 0; i < vkViewports.size(); ++i) {
    vkViewports[i].x     = Viewports[i].X;
    vkViewports[i].width = Viewports[i].Width;

    if (useVulkanNdcYFlip) {
      // vkViewports[i].y      = Viewports[i].Height - Viewports[i].Y; //
      // previous soulution
      vkViewports[i].y
          = Viewports[i].Y + Viewports[i].Height;  // another solution
      // vkViewports[i].y      = Viewports[i].Height;
      vkViewports[i].height = -Viewports[i].Height;
    } else {
      vkViewports[i].y      = Viewports[i].Y;
      vkViewports[i].height = Viewports[i].Height;
    }

    vkViewports[i].minDepth = Viewports[i].MinDepth;
    vkViewports[i].maxDepth = Viewports[i].MaxDepth;
  }

  const auto&           Scissors = PipelineStateFixed->scissors;
  std::vector<VkRect2D> vkScissor;
  vkScissor.resize(Scissors.size());
  for (int32_t i = 0; i < vkScissor.size(); ++i) {
    vkScissor[i].offset.x      = Scissors[i].Offset.x();
    vkScissor[i].offset.y      = Scissors[i].Offset.y();
    vkScissor[i].extent.width  = Scissors[i].Extent.width();
    vkScissor[i].extent.height = Scissors[i].Extent.height();
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
  assert(RenderPass->RenderPassInfo.Subpasses.size() > SubpassIndex);
  const jSubpass& SelectedSubpass
      = RenderPass->RenderPassInfo.Subpasses[SubpassIndex];
  for (int32_t i = 0;
       i < (int32_t)SelectedSubpass.OutputColorAttachments.size();
       ++i) {
    const int32_t AttachmentIndex = SelectedSubpass.OutputColorAttachments[i];
    const bool    IsColorAttachment
        = !RenderPass->RenderPassInfo.Attachments[AttachmentIndex]
               .IsDepthAttachment()
       && !RenderPass->RenderPassInfo.Attachments[AttachmentIndex]
               .IsResolveAttachment();
    if (IsColorAttachment) {
      ++ColorAttachmentCountInSubpass;
    }
  }

  std::vector<VkPipelineColorBlendAttachmentState> ColorBlendAttachmentStates;
  if (ColorAttachmentCountInSubpass > 1) {
    ColorBlendAttachmentStates.resize(
        ColorAttachmentCountInSubpass,
        ((BlendingStateInfoVk*)PipelineStateFixed->BlendingState)
            ->ColorBlendAttachmentInfo);
    colorBlending.attachmentCount = ColorAttachmentCountInSubpass;
    colorBlending.pAttachments    = &ColorBlendAttachmentStates[0];
  } else {
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments
        = &((BlendingStateInfoVk*)PipelineStateFixed->BlendingState)
               ->ColorBlendAttachmentInfo;
  }

  colorBlending.blendConstants[0] = 0.0f;  // Optional
  colorBlending.blendConstants[1] = 0.0f;  // Optional
  colorBlending.blendConstants[2] = 0.0f;  // Optional
  colorBlending.blendConstants[3] = 0.0f;  // Optional

  // 9. Dynamic state
  VkPipelineDynamicStateCreateInfo dynamicState = {};
  dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;

  std::vector<VkDynamicState> dynamicStates;
  if (PipelineStateFixed->DynamicStates.size() > 0) {
    dynamicStates.resize(PipelineStateFixed->DynamicStates.size());

    for (int32_t i = 0; i < (int32_t)PipelineStateFixed->DynamicStates.size();
         ++i) {
      dynamicStates[i]
          = GetVulkanPipelineDynamicState(PipelineStateFixed->DynamicStates[i]);
    }

    dynamicState.dynamicStateCount = (uint32_t)dynamicStates.size();
    dynamicState.pDynamicStates    = &dynamicStates[0];
  }

  // 10. Pipeline layout
  vkPipelineLayout = ShaderBindingLayoutVk::CreatePipelineLayout(
      ShaderBindingLayoutArray, PushConstant);

  VkGraphicsPipelineCreateInfo pipelineInfo = {};
  pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

  // Shader stage
  VkPipelineShaderStageCreateInfo ShaderStages[5];
  uint32_t                        ShaderStageIndex = 0;
  if (GraphicsShader.VertexShader) {
    ShaderStages[ShaderStageIndex++]
        = ((CompiledShaderVk*)GraphicsShader.VertexShader->GetCompiledShader())
              ->ShaderStage;
  }
  if (GraphicsShader.GeometryShader) {
    ShaderStages[ShaderStageIndex++]
        = ((CompiledShaderVk*)GraphicsShader.GeometryShader)->ShaderStage;
  }
  if (GraphicsShader.PixelShader) {
    ShaderStages[ShaderStageIndex++]
        //= ((CompiledShaderVk*)GraphicsShader.PixelShader)->ShaderStage;
        = ((CompiledShaderVk*)GraphicsShader.PixelShader->GetCompiledShader())
			  ->ShaderStage;
  }

  assert(ShaderStageIndex > 0);
  pipelineInfo.stageCount = ShaderStageIndex;
  pipelineInfo.pStages    = &ShaderStages[0];

  // Fixed-function stage
  pipelineInfo.pVertexInputState   = &vertexInputInfo;
  pipelineInfo.pInputAssemblyState = &inputAssembly;
  pipelineInfo.pViewportState      = &viewportState;
  pipelineInfo.pRasterizationState
      = &((RasterizationStateInfoVk*)PipelineStateFixed->RasterizationState)
             ->RasterizationStateInfo;
  pipelineInfo.pMultisampleState
      = &((RasterizationStateInfoVk*)PipelineStateFixed->RasterizationState)
             ->MultisampleStateInfo;
  pipelineInfo.pDepthStencilState
      = &((DepthStencilStateInfoVk*)PipelineStateFixed->DepthStencilState)
             ->DepthStencilStateInfo;
  pipelineInfo.pColorBlendState = &colorBlending;
  pipelineInfo.pDynamicState    = &dynamicState;
  pipelineInfo.layout           = vkPipelineLayout;
  pipelineInfo.renderPass       = (VkRenderPass)RenderPass->GetRenderPass();
  pipelineInfo.subpass          = SubpassIndex;      // index of subpass

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

  if (PipelineStateFixed->IsUseVRS) {
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
                                g_rhi_vk->PipelineCache,
                                1,
                                &pipelineInfo,
                                nullptr,
                                &vkPipeline)
      != VK_SUCCESS) {
    GlobalLogger::Log(LogLevel::Error, "Failed to create graphics pipeline");
    return nullptr;
  }

  size_t hash = GetHash();
  assert(hash);
  // TODO: real time shader update (WIP)

  // if (GraphicsShader.VertexShader) {
  //   Shader::gConnectedPipelineStateHash[GraphicsShader.VertexShader]
  //       .push_back(hash);
  // }
  // if (GraphicsShader.GeometryShader) {
  //   Shader::gConnectedPipelineStateHash[GraphicsShader.GeometryShader]
  //       .push_back(hash);
  // }
  // if (GraphicsShader.PixelShader) {
  //   Shader::gConnectedPipelineStateHash[GraphicsShader.PixelShader]
  //       .push_back(hash);
  // }

  return vkPipeline;
}

void* PipelineStateInfoVk::CreateComputePipelineState() {
  if (vkPipeline) {
    return vkPipeline;
  }

  vkPipelineLayout = ShaderBindingLayoutVk::CreatePipelineLayout(
      ShaderBindingLayoutArray, PushConstant);

  VkComputePipelineCreateInfo computePipelineCreateInfo{};
  computePipelineCreateInfo.sType
      = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
  computePipelineCreateInfo.layout = vkPipelineLayout;
  computePipelineCreateInfo.flags  = 0;
  computePipelineCreateInfo.stage
      = ((CompiledShaderVk*)ComputeShader->GetCompiledShader())->ShaderStage;

  if (vkCreateComputePipelines(g_rhi_vk->m_device_,
                               g_rhi_vk->PipelineCache,
                               1,
                               &computePipelineCreateInfo,
                               nullptr,
                               &vkPipeline)
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

  return vkPipeline;
}

void PipelineStateInfoVk::Bind(
    const std::shared_ptr<jRenderFrameContext>& InRenderFrameContext) const {
  assert(vkPipeline);
  if (PipelineType == EPipelineType::Graphics) {
    vkCmdBindPipeline(
        (VkCommandBuffer)InRenderFrameContext->GetActiveCommandBuffer()
            ->GetNativeHandle(),
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        vkPipeline);
  } else if (PipelineType == EPipelineType::Compute) {
    vkCmdBindPipeline(
        (VkCommandBuffer)InRenderFrameContext->GetActiveCommandBuffer()
            ->GetNativeHandle(),
        VK_PIPELINE_BIND_POINT_COMPUTE,
        vkPipeline);
  } else if (PipelineType == EPipelineType::RayTracing) {
    // TODO: implement
    assert(0);
  }
}

}  // namespace game_engine
