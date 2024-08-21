#ifndef GAME_ENGINE_PIPELINE_STATE_INFO_VK_H
#define GAME_ENGINE_PIPELINE_STATE_INFO_VK_H

#include "gfx/rhi/pipeline_state_info.h"
#include "gfx/rhi/resource_container.h"
#include "gfx/rhi/shader_bindable_resource.h"
#include "gfx/rhi/vulkan/buffer_vk.h"
#include "gfx/rhi/vulkan/render_pass_vk.h"
#include "gfx/rhi/vulkan/render_target_vk.h"
#include "gfx/rhi/vulkan/shader_binding_layout_vk.h"
#include "gfx/rhi/vulkan/shader_vk.h"

#include <math_library/dimension.h>
#include <vulkan/vulkan.h>

#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

namespace game_engine {

// TODO: consider moving to other file
constexpr bool kUseVulkanNdcYFlip = true;


struct SamplerStateInfoVk : public SamplerStateInfo {
  SamplerStateInfoVk() = default;

  SamplerStateInfoVk(const SamplerStateInfo& state)
      : SamplerStateInfo(state) {}

  virtual ~SamplerStateInfoVk() { release(); }

  virtual void initialize() override;
  void         release();

  virtual void* getHandle() const { return m_samplerState_; }

  VkSamplerCreateInfo m_samplerStateInfo_ = {};
  VkSampler           m_samplerState_     = nullptr;
};

struct RasterizationStateInfoVk : public RasterizationStateInfo {
  RasterizationStateInfoVk() = default;

  RasterizationStateInfoVk(const RasterizationStateInfo& state)
      : RasterizationStateInfo(state) {}

  virtual ~RasterizationStateInfoVk() {}

  virtual void initialize() override;

  VkPipelineRasterizationStateCreateInfo m_rasterizationStateInfo_ = {};
  VkPipelineMultisampleStateCreateInfo   m_multisampleStateInfo_   = {};
};

struct StencilOpStateInfoVk : public StencilOpStateInfo {
  StencilOpStateInfoVk() = default;

  StencilOpStateInfoVk(const StencilOpStateInfo& state)
      : StencilOpStateInfo(state) {}

  virtual ~StencilOpStateInfoVk() {}

  virtual void initialize() override;

  VkStencilOpState m_stencilOpStateInfo_ = {};
};

struct DepthStencilStateInfoVk : public DepthStencilStateInfo {
  DepthStencilStateInfoVk() = default;

  DepthStencilStateInfoVk(const DepthStencilStateInfo& state)
      : DepthStencilStateInfo(state) {}

  virtual ~DepthStencilStateInfoVk() {}

  virtual void initialize() override;

  VkPipelineDepthStencilStateCreateInfo m_depthStencilStateInfo_ = {};
};

struct BlendingStateInfoVk : public BlendingStateInfo {
  BlendingStateInfoVk() = default;

  BlendingStateInfoVk(const BlendingStateInfo& state)
      : BlendingStateInfo(state) {}

  virtual ~BlendingStateInfoVk() {}

  virtual void initialize() override;

  VkPipelineColorBlendAttachmentState m_colorBlendAttachmentInfo_ = {};
};

//////////////////////////////////////////////////////////////////////////
// PipelineStateInfoVk
//////////////////////////////////////////////////////////////////////////
struct PipelineStateInfoVk : public PipelineStateInfo {
  PipelineStateInfoVk() = default;

  PipelineStateInfoVk(
      const PipelineStateFixedInfo*   pipelineStateFixed,
      const GraphicsPipelineShader    shader,
      const VertexBufferArray&        vertexBufferArray,
      const RenderPass*               renderPass,
      const ShaderBindingLayoutArray& shaderBindingLayoutArray,
      const PushConstant*             pushConstant)
      : PipelineStateInfo(pipelineStateFixed,
                           shader,
                           vertexBufferArray,
                           renderPass,
                           shaderBindingLayoutArray,
                           pushConstant) {}

  PipelineStateInfoVk(const PipelineStateInfo& pipelineState)
      : PipelineStateInfo(pipelineState) {}

  PipelineStateInfoVk(PipelineStateInfo&& pipelineState)
      : PipelineStateInfo(std::move(pipelineState)) {}

  virtual ~PipelineStateInfoVk() { release(); }

  void release();

  virtual void initialize() override;

  virtual void* getHandle() const override { return m_pipeline_; }

  virtual void* getPipelineLayoutHandle() const override {
    return m_pipelineLayout_;
  }

  virtual void* createGraphicsPipelineState() override;
  virtual void* createComputePipelineState() override;
  // TODO: implement
  //virtual void* createRaytracingPipelineState() override;
  virtual void  bind(const std::shared_ptr<RenderFrameContext>&
                         renderFrameContext) const override;

  VkPipeline       m_pipeline_ = nullptr;
  VkPipelineLayout m_pipelineLayout_
      = nullptr;  // The PipelineLayout is cached in the PipelineLayoutPool, so
                  // it is not destroyed.

  // TODO: currently not used
  // Raytracing ShaderTables
  //std::shared_ptr<BufferVk> RaygenBuffer;
  //std::shared_ptr<BufferVk> MissBuffer;
  //std::shared_ptr<BufferVk> HitGroupBuffer;
  //VkStridedDeviceAddressRegionKHR RaygenStridedDeviceAddressRegion{};
  //VkStridedDeviceAddressRegionKHR MissStridedDeviceAddressRegion{};
  //VkStridedDeviceAddressRegionKHR HitStridedDeviceAddressRegion{};
};

}  // namespace game_engine

#endif  // GAME_ENGINE_PIPELINE_STATE_INFO_VK_H
