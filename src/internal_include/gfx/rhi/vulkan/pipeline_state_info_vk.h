#ifndef GAME_ENGINE_PIPELINE_STATE_INFO_VK_H
#define GAME_ENGINE_PIPELINE_STATE_INFO_VK_H

#include "gfx/rhi/pipeline_state_info.h"
#include "gfx/rhi/resource_container.h"
#include "gfx/rhi/shader_bindable_resource.h"
#include "gfx/rhi/vulkan/buffer_vk.h"
#include "gfx/rhi/vulkan/render_pass_vk.h"
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
constexpr bool g_kUseVulkanNdcYFlip = true;

struct SamplerStateInfoVk : public SamplerStateInfo {
  // ======= BEGIN: public constructors =======================================

  SamplerStateInfoVk() = default;

  SamplerStateInfoVk(const SamplerStateInfo& state)
      : SamplerStateInfo(state) {}

  // ======= END: public constructors   =======================================

  // ======= BEGIN: public destructor =========================================

  virtual ~SamplerStateInfoVk() { release(); }

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overridden methods =================================

  virtual void initialize() override;

  virtual void* getHandle() const { return m_samplerState_; }

  // ======= END: public overridden methods   =================================

  // ======= BEGIN: public misc methods =======================================

  void release();

  // ======= END: public misc methods   =======================================

  // ======= BEGIN: public misc fields ========================================

  VkSamplerCreateInfo m_samplerStateInfo_ = {};
  VkSampler           m_samplerState_     = nullptr;

  // ======= END: public misc fields   ========================================
};

struct RasterizationStateInfoVk : public RasterizationStateInfo {
  // ======= BEGIN: public constructors =======================================

  RasterizationStateInfoVk() = default;

  RasterizationStateInfoVk(const RasterizationStateInfo& state)
      : RasterizationStateInfo(state) {}

  // ======= END: public constructors   =======================================

  // ======= BEGIN: public destructor =========================================

  virtual ~RasterizationStateInfoVk() {}

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overridden methods =================================

  virtual void initialize() override;

  // ======= END: public overridden methods   =================================

  // ======= BEGIN: public misc fields ========================================

  VkPipelineRasterizationStateCreateInfo m_rasterizationStateInfo_ = {};
  VkPipelineMultisampleStateCreateInfo   m_multisampleStateInfo_   = {};

  // ======= END: public misc fields   ========================================
};

struct StencilOpStateInfoVk : public StencilOpStateInfo {
  // ======= BEGIN: public constructors =======================================

  StencilOpStateInfoVk() = default;

  StencilOpStateInfoVk(const StencilOpStateInfo& state)
      : StencilOpStateInfo(state) {}

  // ======= END: public constructors   =======================================

  // ======= BEGIN: public destructor =========================================

  virtual ~StencilOpStateInfoVk() {}

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overridden methods =================================

  virtual void initialize() override;

  // ======= END: public overridden methods   =================================

  // ======= BEGIN: public misc fields ========================================

  VkStencilOpState m_stencilOpStateInfo_ = {};

  // ======= END: public misc fields   ========================================
};

struct DepthStencilStateInfoVk : public DepthStencilStateInfo {
  // ======= BEGIN: public constructors =======================================

  DepthStencilStateInfoVk() = default;

  DepthStencilStateInfoVk(const DepthStencilStateInfo& state)
      : DepthStencilStateInfo(state) {}

  // ======= END: public constructors   =======================================

  // ======= BEGIN: public destructor =========================================

  virtual ~DepthStencilStateInfoVk() {}

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overridden methods =================================

  virtual void initialize() override;

  // ======= END: public overridden methods   =================================

  // ======= BEGIN: public misc fields ========================================

  VkPipelineDepthStencilStateCreateInfo m_depthStencilStateInfo_ = {};

  // ======= END: public misc fields   ========================================
};

struct BlendingStateInfoVk : public BlendingStateInfo {
  // ======= BEGIN: public constructors =======================================

  BlendingStateInfoVk() = default;

  BlendingStateInfoVk(const BlendingStateInfo& state)
      : BlendingStateInfo(state) {}

  // ======= END: public constructors   =======================================

  // ======= BEGIN: public destructor =========================================

  virtual ~BlendingStateInfoVk() {}

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overridden methods =================================

  virtual void initialize() override;

  // ======= END: public overridden methods   =================================

  // ======= BEGIN: public misc fields ========================================

  VkPipelineColorBlendAttachmentState m_colorBlendAttachmentInfo_ = {};

  // ======= END: public misc fields   ========================================
};

//////////////////////////////////////////////////////////////////////////
// PipelineStateInfoVk
//////////////////////////////////////////////////////////////////////////
struct PipelineStateInfoVk : public PipelineStateInfo {
  // ======= BEGIN: public constructors =======================================

  PipelineStateInfoVk() = default;

  PipelineStateInfoVk(const PipelineStateFixedInfo*   pipelineStateFixed,
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

  // ======= END: public constructors   =======================================

  // ======= BEGIN: public destructor =========================================

  virtual ~PipelineStateInfoVk() { release(); }

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overridden methods =================================

  virtual void initialize() override;

  virtual void* createGraphicsPipelineState() override;
  virtual void* createComputePipelineState() override;
  // TODO: implement
  // virtual void* createRaytracingPipelineState() override;
  virtual void  bind(
       const std::shared_ptr<CommandBuffer>& commandBuffer) const override;

  virtual void* getHandle() const override { return m_pipeline_; }

  virtual void* getPipelineLayoutHandle() const override {
    return m_pipelineLayout_;
  }

  // ======= END: public overridden methods   =================================

  // ======= BEGIN: public misc methods =======================================

  void release();

  // ======= END: public misc methods   =======================================

  // ======= BEGIN: public misc fields ========================================

  VkPipeline       m_pipeline_ = nullptr;
  VkPipelineLayout m_pipelineLayout_
      = nullptr;  // The PipelineLayout is cached in the PipelineLayoutPool, so
                  // it is not destroyed.

  // TODO: currently not used
  // Raytracing ShaderTables
  // std::shared_ptr<BufferVk> RaygenBuffer;
  // std::shared_ptr<BufferVk> MissBuffer;
  // std::shared_ptr<BufferVk> HitGroupBuffer;
  // VkStridedDeviceAddressRegionKHR RaygenStridedDeviceAddressRegion{};
  // VkStridedDeviceAddressRegionKHR MissStridedDeviceAddressRegion{};
  // VkStridedDeviceAddressRegionKHR HitStridedDeviceAddressRegion{};

  // ======= END: public misc fields   ========================================
};

}  // namespace game_engine

#endif  // GAME_ENGINE_PIPELINE_STATE_INFO_VK_H
