#ifndef GAME_ENGINE_PIPELINE_STATE_INFO_DX12_H
#define GAME_ENGINE_PIPELINE_STATE_INFO_DX12_H

#include "gfx/rhi/dx12/descriptor_heap_dx12.h"
#include "gfx/rhi/pipeline_state_info.h"
#include "platform/windows/windows_platform_setup.h"

namespace game_engine {

struct CommandBufferDx12;

struct SamplerStateInfoDx12 : public SamplerStateInfo {
  SamplerStateInfoDx12() = default;

  SamplerStateInfoDx12(const SamplerStateInfo& state)
      : SamplerStateInfo(state) {}

  virtual ~SamplerStateInfoDx12() { release(); }

  virtual void initialize() override;
  void         release();

  virtual void* getHandle() const { return (void*)&m_samplerDesc_; }

  D3D12_SAMPLER_DESC m_samplerDesc_;
  DescriptorDx12     m_samplerSRV_;
};

struct RasterizationStateInfoDx12 : public RasterizationStateInfo {
  RasterizationStateInfoDx12() = default;

  RasterizationStateInfoDx12(const RasterizationStateInfo& state)
      : RasterizationStateInfo(state) {}

  virtual ~RasterizationStateInfoDx12() {}

  virtual void initialize() override;

  D3D12_RASTERIZER_DESC m_rasterizeDesc_   = {};
  DXGI_SAMPLE_DESC      m_multiSampleDesc_ = {};
};

struct StencilOpStateInfoDx12 : public StencilOpStateInfo {
  StencilOpStateInfoDx12() = default;

  StencilOpStateInfoDx12(const StencilOpStateInfo& state)
      : StencilOpStateInfo(state) {}

  virtual ~StencilOpStateInfoDx12() {}

  virtual void initialize() override;

  D3D12_DEPTH_STENCILOP_DESC m_stencilOpStateDesc_ = {};
};

struct DepthStencilStateInfoDx12 : public DepthStencilStateInfo {
  DepthStencilStateInfoDx12() = default;

  DepthStencilStateInfoDx12(const DepthStencilStateInfo& state)
      : DepthStencilStateInfo(state) {}

  virtual ~DepthStencilStateInfoDx12() {}

  virtual void initialize() override;

  D3D12_DEPTH_STENCIL_DESC m_depthStencilStateDesc_ = {};
};

struct BlendingStateInfoDx12 : public BlendingStateInfo {
  BlendingStateInfoDx12() = default;

  BlendingStateInfoDx12(const BlendingStateInfo& state)
      : BlendingStateInfo(state) {}

  virtual ~BlendingStateInfoDx12() {}

  virtual void initialize() override;

  D3D12_RENDER_TARGET_BLEND_DESC m_blendDesc_ = {};
};

//////////////////////////////////////////////////////////////////////////
// PipelineStateInfoDx12
//////////////////////////////////////////////////////////////////////////
struct PipelineStateInfoDx12 : public PipelineStateInfo {
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

  virtual ~PipelineStateInfoDx12() { release(); }

  void release();

  virtual void initialize() override;

  virtual void* getHandle() const override { return nullptr; }

  virtual void* getPipelineLayoutHandle() const override { return nullptr; }

  virtual void* createGraphicsPipelineState() override;
  virtual void* createComputePipelineState() override;
  // virtual void* createRaytracingPipelineState() override;
  virtual void  bind(const std::shared_ptr<RenderFrameContext>&
                         renderFrameContext) const override;
  void          bind(CommandBufferDx12* commandList) const;

  ComPtr<ID3D12PipelineState> m_pipelineState_;
  std::vector<D3D12_VIEWPORT> m_viewports_;
  std::vector<D3D12_RECT>     m_scissors_;

  // RaytracingStateObject
  ComPtr<ID3D12StateObject> m_raytracingStateObject_;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_PIPELINE_STATE_INFO_DX12_H