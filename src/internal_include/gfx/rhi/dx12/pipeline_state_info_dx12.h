#ifndef GAME_ENGINE_PIPELINE_STATE_INFO_DX12_H
#define GAME_ENGINE_PIPELINE_STATE_INFO_DX12_H

#include "gfx/rhi/dx12/descriptor_heap_dx12.h"
#include "gfx/rhi/pipeline_state_info.h"
#include "platform/windows/windows_platform_setup.h"

namespace game_engine {

struct jCommandBuffer_DX12;

struct jSamplerStateInfo_DX12 : public jSamplerStateInfo {
  jSamplerStateInfo_DX12() = default;

  jSamplerStateInfo_DX12(const jSamplerStateInfo& state)
      : jSamplerStateInfo(state) {}

  virtual ~jSamplerStateInfo_DX12() { Release(); }

  virtual void Initialize() override;
  void         Release();

  virtual void* GetHandle() const { return (void*)&SamplerDesc; }

  D3D12_SAMPLER_DESC SamplerDesc;
  jDescriptor_DX12   SamplerSRV;
};

struct jRasterizationStateInfo_DX12 : public jRasterizationStateInfo {
  jRasterizationStateInfo_DX12() = default;

  jRasterizationStateInfo_DX12(const jRasterizationStateInfo& state)
      : jRasterizationStateInfo(state) {}

  virtual ~jRasterizationStateInfo_DX12() {}

  virtual void Initialize() override;

  D3D12_RASTERIZER_DESC RasterizeDesc   = {};
  DXGI_SAMPLE_DESC      MultiSampleDesc = {};
};

struct jStencilOpStateInfo_DX12 : public jStencilOpStateInfo {
  jStencilOpStateInfo_DX12() = default;

  jStencilOpStateInfo_DX12(const jStencilOpStateInfo& state)
      : jStencilOpStateInfo(state) {}

  virtual ~jStencilOpStateInfo_DX12() {}

  virtual void Initialize() override;

  D3D12_DEPTH_STENCILOP_DESC StencilOpStateDesc = {};
};

struct jDepthStencilStateInfo_DX12 : public jDepthStencilStateInfo {
  jDepthStencilStateInfo_DX12() = default;

  jDepthStencilStateInfo_DX12(const jDepthStencilStateInfo& state)
      : jDepthStencilStateInfo(state) {}

  virtual ~jDepthStencilStateInfo_DX12() {}

  virtual void Initialize() override;

  D3D12_DEPTH_STENCIL_DESC DepthStencilStateDesc = {};
};

struct jBlendingStateInfo_DX12 : public jBlendingStateInfo {
  jBlendingStateInfo_DX12() = default;

  jBlendingStateInfo_DX12(const jBlendingStateInfo& state)
      : jBlendingStateInfo(state) {}

  virtual ~jBlendingStateInfo_DX12() {}

  virtual void Initialize() override;

  D3D12_RENDER_TARGET_BLEND_DESC BlendDesc = {};
};

//////////////////////////////////////////////////////////////////////////
// jPipelineStateInfo_DX12
//////////////////////////////////////////////////////////////////////////
struct jPipelineStateInfo_DX12 : public jPipelineStateInfo {
  jPipelineStateInfo_DX12() = default;

  jPipelineStateInfo_DX12(
      const jPipelineStateFixedInfo*   pipelineStateFixed,
      const GraphicsPipelineShader     shader,
      const jVertexBufferArray&        InVertexBufferArray,
      const jRenderPass*               renderPass,
      const jShaderBindingLayoutArray& InShaderBindingLayoutArray,
      const jPushConstant*             pushConstant)
      : jPipelineStateInfo(pipelineStateFixed,
                           shader,
                           InVertexBufferArray,
                           renderPass,
                           InShaderBindingLayoutArray,
                           pushConstant) {}

  jPipelineStateInfo_DX12(const jPipelineStateInfo& pipelineState)
      : jPipelineStateInfo(pipelineState) {}

  jPipelineStateInfo_DX12(jPipelineStateInfo&& pipelineState)
      : jPipelineStateInfo(std::move(pipelineState)) {}

  virtual ~jPipelineStateInfo_DX12() { Release(); }

  void Release();

  virtual void Initialize() override;

  virtual void* GetHandle() const override { return nullptr; }

  virtual void* GetPipelineLayoutHandle() const override { return nullptr; }

  virtual void* CreateGraphicsPipelineState() override;
  virtual void* CreateComputePipelineState() override;
  // virtual void* CreateRaytracingPipelineState() override;
  virtual void  Bind(const std::shared_ptr<jRenderFrameContext>&
                         InRenderFrameContext) const override;
  void          Bind(jCommandBuffer_DX12* InCommandList) const;

  ComPtr<ID3D12PipelineState> PipelineState;
  std::vector<D3D12_VIEWPORT> Viewports;
  std::vector<D3D12_RECT>     Scissors;

  // RaytracingStateObject
  ComPtr<ID3D12StateObject> RaytracingStateObject;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_PIPELINE_STATE_INFO_DX12_H