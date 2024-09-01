#ifndef GAME_ENGINE_PIPELINE_STATE_INFO_DX12_H
#define GAME_ENGINE_PIPELINE_STATE_INFO_DX12_H

#include "gfx/rhi/dx12/descriptor_heap_dx12.h"
#include "gfx/rhi/pipeline_state_info.h"
#include "platform/windows/windows_platform_setup.h"

namespace game_engine {

struct CommandBufferDx12;

struct SamplerStateInfoDx12 : public SamplerStateInfo {
  // ======= BEGIN: public constructors =======================================

  SamplerStateInfoDx12() = default;

  SamplerStateInfoDx12(const SamplerStateInfo& state)
      : SamplerStateInfo(state) {}

  // ======= END: public constructors   =======================================

  // ======= BEGIN: public destructor =========================================

  virtual ~SamplerStateInfoDx12() { release(); }

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overridden methods =================================

  virtual void initialize() override;

  virtual void* getHandle() const { return (void*)&m_samplerDesc_; }

  // ======= END: public overridden methods   =================================

  // ======= BEGIN: public misc methods =======================================

  void release();

  // ======= END: public misc methods   =======================================

  // ======= BEGIN: public misc fields ========================================

  D3D12_SAMPLER_DESC m_samplerDesc_;
  DescriptorDx12     m_samplerSRV_;

  // ======= END: public misc fields   ========================================
};

struct RasterizationStateInfoDx12 : public RasterizationStateInfo {
  // ======= BEGIN: public constructors =======================================

  RasterizationStateInfoDx12() = default;

  RasterizationStateInfoDx12(const RasterizationStateInfo& state)
      : RasterizationStateInfo(state) {}

  // ======= END: public constructors   =======================================

  // ======= BEGIN: public destructor =========================================

  virtual ~RasterizationStateInfoDx12() {}

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overridden methods =================================

  virtual void initialize() override;

  // ======= END: public overridden methods   =================================

  // ======= BEGIN: public misc fields ========================================

  D3D12_RASTERIZER_DESC m_rasterizeDesc_   = {};
  DXGI_SAMPLE_DESC      m_multiSampleDesc_ = {};

  // ======= END: public misc fields   ========================================
};

struct StencilOpStateInfoDx12 : public StencilOpStateInfo {
  // ======= BEGIN: public constructors =======================================

  StencilOpStateInfoDx12() = default;

  StencilOpStateInfoDx12(const StencilOpStateInfo& state)
      : StencilOpStateInfo(state) {}

  // ======= END: public constructors   =======================================

  // ======= BEGIN: public destructor =========================================

  virtual ~StencilOpStateInfoDx12() {}

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overridden methods =================================

  virtual void initialize() override;

  // ======= END: public overridden methods   =================================

  // ======= BEGIN: public misc fields ========================================

  D3D12_DEPTH_STENCILOP_DESC m_stencilOpStateDesc_ = {};

  // ======= END: public misc fields   ========================================
};

struct DepthStencilStateInfoDx12 : public DepthStencilStateInfo {
  // ======= BEGIN: public constructors =======================================

  DepthStencilStateInfoDx12() = default;

  DepthStencilStateInfoDx12(const DepthStencilStateInfo& state)
      : DepthStencilStateInfo(state) {}

  // ======= END: public constructors   =======================================

  // ======= BEGIN: public destructor =========================================

  virtual ~DepthStencilStateInfoDx12() {}

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overridden methods =================================

  virtual void initialize() override;

  // ======= END: public overridden methods   =================================

  // ======= BEGIN: public misc fields ========================================

  D3D12_DEPTH_STENCIL_DESC m_depthStencilStateDesc_ = {};

  // ======= END: public misc fields   ========================================
};

struct BlendingStateInfoDx12 : public BlendingStateInfo {
  // ======= BEGIN: public constructors =======================================

  BlendingStateInfoDx12() = default;

  BlendingStateInfoDx12(const BlendingStateInfo& state)
      : BlendingStateInfo(state) {}

  // ======= END: public constructors   =======================================

  // ======= BEGIN: public destructor =========================================

  virtual ~BlendingStateInfoDx12() {}

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overridden methods =================================

  virtual void initialize() override;

  // ======= END: public overridden methods   =================================

  // ======= BEGIN: public misc fields ========================================

  D3D12_RENDER_TARGET_BLEND_DESC m_blendDesc_ = {};

  // ======= END: public misc fields   ========================================
};

//////////////////////////////////////////////////////////////////////////
// PipelineStateInfoDx12
//////////////////////////////////////////////////////////////////////////
struct PipelineStateInfoDx12 : public PipelineStateInfo {
  // ======= BEGIN: public constructors =======================================

  PipelineStateInfoDx12() = default;

  PipelineStateInfoDx12(
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

  PipelineStateInfoDx12(const PipelineStateInfo& pipelineState)
      : PipelineStateInfo(pipelineState) {}

  PipelineStateInfoDx12(PipelineStateInfo&& pipelineState)
      : PipelineStateInfo(std::move(pipelineState)) {}

  // ======= END: public constructors   =======================================

  // ======= BEGIN: public destructor =========================================

  virtual ~PipelineStateInfoDx12() { release(); }

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overridden methods =================================

  virtual void initialize() override;

  virtual void* createGraphicsPipelineState() override;
  virtual void* createComputePipelineState() override;
  // virtual void* createRaytracingPipelineState() override;
  virtual void  bind(const std::shared_ptr<RenderFrameContext>&
                         renderFrameContext) const override;

  virtual void* getHandle() const override { return nullptr; }

  virtual void* getPipelineLayoutHandle() const override { return nullptr; }

  // ======= END: public overridden methods   =================================

  // ======= BEGIN: public misc methods =======================================



  // ======= END: public misc methods   =======================================

  void bind(CommandBufferDx12* commandList) const;

  void release();

  // ======= BEGIN: public misc fields ========================================

  ComPtr<ID3D12PipelineState> m_pipelineState_;
  std::vector<D3D12_VIEWPORT> m_viewports_;
  std::vector<D3D12_RECT>     m_scissors_;

  // TODO: currently not used
  //ComPtr<ID3D12StateObject> m_raytracingStateObject_;

  // ======= END: public misc fields   ========================================




};

}  // namespace game_engine

#endif  // GAME_ENGINE_PIPELINE_STATE_INFO_DX12_H