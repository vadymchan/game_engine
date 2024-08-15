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

  virtual ~SamplerStateInfoDx12() { Release(); }

  virtual void Initialize() override;
  void         Release();

  virtual void* GetHandle() const { return (void*)&SamplerDesc; }

  D3D12_SAMPLER_DESC SamplerDesc;
  DescriptorDx12   SamplerSRV;
};

struct RasterizationStateInfoDx12 : public RasterizationStateInfo {
  RasterizationStateInfoDx12() = default;

  RasterizationStateInfoDx12(const RasterizationStateInfo& state)
      : RasterizationStateInfo(state) {}

  virtual ~RasterizationStateInfoDx12() {}

  virtual void Initialize() override;

  D3D12_RASTERIZER_DESC RasterizeDesc   = {};
  DXGI_SAMPLE_DESC      MultiSampleDesc = {};
};

struct StencilOpStateInfoDx12 : public StencilOpStateInfo {
  StencilOpStateInfoDx12() = default;

  StencilOpStateInfoDx12(const StencilOpStateInfo& state)
      : StencilOpStateInfo(state) {}

  virtual ~StencilOpStateInfoDx12() {}

  virtual void Initialize() override;

  D3D12_DEPTH_STENCILOP_DESC StencilOpStateDesc = {};
};

struct DepthStencilStateInfoDx12 : public DepthStencilStateInfo {
  DepthStencilStateInfoDx12() = default;

  DepthStencilStateInfoDx12(const DepthStencilStateInfo& state)
      : DepthStencilStateInfo(state) {}

  virtual ~DepthStencilStateInfoDx12() {}

  virtual void Initialize() override;

  D3D12_DEPTH_STENCIL_DESC DepthStencilStateDesc = {};
};

struct BlendingStateInfoDx12 : public BlendingStateInfo {
  BlendingStateInfoDx12() = default;

  BlendingStateInfoDx12(const BlendingStateInfo& state)
      : BlendingStateInfo(state) {}

  virtual ~BlendingStateInfoDx12() {}

  virtual void Initialize() override;

  D3D12_RENDER_TARGET_BLEND_DESC BlendDesc = {};
};

//////////////////////////////////////////////////////////////////////////
// PipelineStateInfoDx12
//////////////////////////////////////////////////////////////////////////
struct PipelineStateInfoDx12 : public PipelineStateInfo {
  PipelineStateInfoDx12() = default;

  PipelineStateInfoDx12(
      const PipelineStateFixedInfo*   pipelineStateFixed,
      const GraphicsPipelineShader     shader,
      const VertexBufferArray&        InVertexBufferArray,
      const RenderPass*               renderPass,
      const ShaderBindingLayoutArray& InShaderBindingLayoutArray,
      const PushConstant*             pushConstant)
      : PipelineStateInfo(pipelineStateFixed,
                           shader,
                           InVertexBufferArray,
                           renderPass,
                           InShaderBindingLayoutArray,
                           pushConstant) {}

  PipelineStateInfoDx12(const PipelineStateInfo& pipelineState)
      : PipelineStateInfo(pipelineState) {}

  PipelineStateInfoDx12(PipelineStateInfo&& pipelineState)
      : PipelineStateInfo(std::move(pipelineState)) {}

  virtual ~PipelineStateInfoDx12() { Release(); }

  void Release();

  virtual void Initialize() override;

  virtual void* GetHandle() const override { return nullptr; }

  virtual void* GetPipelineLayoutHandle() const override { return nullptr; }

  virtual void* CreateGraphicsPipelineState() override;
  virtual void* CreateComputePipelineState() override;
  // virtual void* CreateRaytracingPipelineState() override;
  virtual void  Bind(const std::shared_ptr<RenderFrameContext>&
                         InRenderFrameContext) const override;
  void          Bind(CommandBufferDx12* InCommandList) const;

  ComPtr<ID3D12PipelineState> PipelineState;
  std::vector<D3D12_VIEWPORT> Viewports;
  std::vector<D3D12_RECT>     Scissors;

  // RaytracingStateObject
  ComPtr<ID3D12StateObject> RaytracingStateObject;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_PIPELINE_STATE_INFO_DX12_H