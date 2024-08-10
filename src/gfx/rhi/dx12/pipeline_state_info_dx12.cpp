#include "gfx/rhi/dx12/pipeline_state_info_dx12.h"

#include "gfx/rhi/dx12/buffer_dx12.h"
#include "gfx/rhi/dx12/command_list_dx12.h"
#include "gfx/rhi/dx12/render_pass_dx12.h"
#include "gfx/rhi/dx12/rhi_dx12.h"
#include "gfx/rhi/dx12/shader_binding_layout_dx12.h"
#include "gfx/rhi/dx12/shader_dx12.h"
#include "gfx/rhi/dx12/utils_dx12.h"

#include <algorithm>

namespace game_engine {

void jSamplerStateInfo_DX12::Initialize() {
  D3D12_SAMPLER_DESC samplerDesc = {};

  samplerDesc.AddressU      = GetDX12TextureAddressMode(AddressU);
  samplerDesc.AddressV      = GetDX12TextureAddressMode(AddressV);
  samplerDesc.AddressW      = GetDX12TextureAddressMode(AddressW);
  samplerDesc.MinLOD        = MinLOD;
  samplerDesc.MaxLOD        = MaxLOD;
  samplerDesc.MipLODBias    = MipLODBias;
  samplerDesc.MaxAnisotropy = (uint32_t)MaxAnisotropy;
  if (MaxAnisotropy > 1) {
    samplerDesc.Filter = D3D12_FILTER_ANISOTROPIC;
  } else {
    samplerDesc.Filter = GetDX12TextureFilter(
        Minification, Magnification, IsEnableComparisonMode);
  }
  samplerDesc.ComparisonFunc = GetDX12CompareOp(ComparisonFunc);

  assert(sizeof(samplerDesc.BorderColor) == sizeof(BorderColor));
  memcpy(samplerDesc.BorderColor, &BorderColor, sizeof(BorderColor));

  SamplerSRV = g_rhi_dx12->SamplerDescriptorHeaps.Alloc();

  assert(g_rhi_dx12);
  assert(g_rhi_dx12->Device);
  g_rhi_dx12->Device->CreateSampler(&samplerDesc, SamplerSRV.CPUHandle);

  ResourceName = Name(ToString().c_str());
}

void jRasterizationStateInfo_DX12::Initialize() {
  RasterizeDesc.FillMode              = GetDX12PolygonMode(PolygonMode);
  RasterizeDesc.CullMode              = GetDX12CullMode(CullMode);
  RasterizeDesc.FrontCounterClockwise = FrontFace == EFrontFace::CCW;
  RasterizeDesc.DepthBias
      = (int32_t)(DepthBiasEnable ? DepthBiasConstantFactor : 0);
  RasterizeDesc.DepthBiasClamp       = DepthBiasClamp;
  RasterizeDesc.SlopeScaledDepthBias = DepthBiasSlopeFactor;
  RasterizeDesc.DepthClipEnable      = DepthClampEnable;
  RasterizeDesc.MultisampleEnable    = (int32_t)SampleCount > 1;

  // RasterizeDesc.AntialiasedLineEnable;
  // RasterizeDesc.ForcedSampleCount;
  // RasterizeDesc.ConservativeRaster;

  MultiSampleDesc.Count = (int32_t)SampleCount;
}

void jStencilOpStateInfo_DX12::Initialize() {
  StencilOpStateDesc.StencilDepthFailOp = GetDX12StencilOp(DepthFailOp);
  StencilOpStateDesc.StencilFailOp      = GetDX12StencilOp(FailOp);
  StencilOpStateDesc.StencilFunc        = GetDX12CompareOp(CompareOp);
  StencilOpStateDesc.StencilPassOp      = GetDX12StencilOp(PassOp);
}

void jDepthStencilStateInfo_DX12::Initialize() {
  DepthStencilStateDesc.DepthEnable    = DepthTestEnable;
  DepthStencilStateDesc.DepthWriteMask = DepthWriteEnable
                                           ? D3D12_DEPTH_WRITE_MASK_ALL
                                           : D3D12_DEPTH_WRITE_MASK_ZERO;
  DepthStencilStateDesc.DepthFunc      = GetDX12CompareOp(DepthCompareOp);
  DepthStencilStateDesc.StencilEnable  = StencilTestEnable;
  // DepthStencilStateDesc.StencilReadMask;
  // DepthStencilStateDesc.StencilWriteMask;
  if (Front) {
    DepthStencilStateDesc.FrontFace
        = ((jStencilOpStateInfo_DX12*)Front)->StencilOpStateDesc;
  }

  if (Back) {
    DepthStencilStateDesc.BackFace
        = ((jStencilOpStateInfo_DX12*)Back)->StencilOpStateDesc;
  }

  // MinDepthBounds;
  // MaxDepthBounds;
}

void jBlendingStateInfo_DX12::Initialize() {
  BlendDesc.BlendEnable           = BlendEnable;
  BlendDesc.SrcBlend              = GetDX12BlendFactor(Src);
  BlendDesc.DestBlend             = GetDX12BlendFactor(Dest);
  BlendDesc.BlendOp               = GetDX12BlendOp(BlendOp);
  BlendDesc.SrcBlendAlpha         = GetDX12BlendFactor(SrcAlpha);
  BlendDesc.DestBlendAlpha        = GetDX12BlendFactor(DestAlpha);
  BlendDesc.BlendOpAlpha          = GetDX12BlendOp(AlphaBlendOp);
  BlendDesc.RenderTargetWriteMask = GetDX12ColorMask(ColorWriteMask);

  // BlendDesc.LogicOpEnable;
  // BlendDesc.LogicOp;
}

void jSamplerStateInfo_DX12::Release() {
  SamplerSRV.Free();
}

void jPipelineStateInfo_DX12::Release() {
}

void jPipelineStateInfo_DX12::Initialize() {
  if (PipelineType == EPipelineType::Graphics) {
    CreateGraphicsPipelineState();
  } else if (PipelineType == EPipelineType::Compute) {
    CreateComputePipelineState();
  } else if (PipelineType == EPipelineType::RayTracing) {
    CreateRaytracingPipelineState();
  } else {
    assert(0);
  }
}

void* jPipelineStateInfo_DX12::CreateGraphicsPipelineState() {
  PipelineState = nullptr;

  D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};

  assert(VertexBufferArray.NumOfData > 0);

  std::vector<D3D12_INPUT_ELEMENT_DESC> OutInputElementDescs;
  jVertexBuffer_DX12::CreateVertexInputState(OutInputElementDescs,
                                             VertexBufferArray);
  psoDesc.InputLayout.pInputElementDescs = OutInputElementDescs.data();
  psoDesc.InputLayout.NumElements = (uint32_t)OutInputElementDescs.size();

  // In DX12, only one can be used, but this will be changed. We need to decide
  // whether to use one or multiple.
  ComPtr<ID3D12RootSignature> RootSignature
      = jShaderBindingLayout_DX12::CreateRootSignature(
          ShaderBindingLayoutArray);
  psoDesc.pRootSignature = RootSignature.Get();

  if (GraphicsShader.VertexShader) {
    auto VS_Compiled = (jCompiledShader_DX12*)
                           GraphicsShader.VertexShader->GetCompiledShader();
    assert(VS_Compiled);
    psoDesc.VS
        = {.pShaderBytecode = VS_Compiled->ShaderBlob->GetBufferPointer(),
           .BytecodeLength  = VS_Compiled->ShaderBlob->GetBufferSize()};
  }
  if (GraphicsShader.GeometryShader) {
    auto GS_Compiled = (jCompiledShader_DX12*)
                           GraphicsShader.GeometryShader->GetCompiledShader();
    assert(GS_Compiled);
    psoDesc.GS
        = {.pShaderBytecode = GS_Compiled->ShaderBlob->GetBufferPointer(),
           .BytecodeLength  = GS_Compiled->ShaderBlob->GetBufferSize()};
  }
  if (GraphicsShader.PixelShader) {
    auto PS_Compiled = (jCompiledShader_DX12*)
                           GraphicsShader.PixelShader->GetCompiledShader();
    assert(PS_Compiled);
    psoDesc.PS
        = {.pShaderBytecode = PS_Compiled->ShaderBlob->GetBufferPointer(),
           .BytecodeLength  = PS_Compiled->ShaderBlob->GetBufferSize()};
  }

  psoDesc.RasterizerState
      = ((jRasterizationStateInfo_DX12*)PipelineStateFixed->RasterizationState)
            ->RasterizeDesc;
  psoDesc.SampleDesc.Count
      = ((jRasterizationStateInfo_DX12*)PipelineStateFixed->RasterizationState)
            ->MultiSampleDesc.Count;

  jRenderPass_DX12* RenderPassDX12 = (jRenderPass_DX12*)RenderPass;

  // Should we specify the blending operation separately? Let's check the
  // current support of RenderPass.
  for (int32_t i = 0; i < (int32_t)RenderPassDX12->GetRTVFormats().size();
       ++i) {
    psoDesc.BlendState.RenderTarget[i]
        = ((jBlendingStateInfo_DX12*)PipelineStateFixed->BlendingState)
              ->BlendDesc;
  }
  psoDesc.DepthStencilState
      = ((jDepthStencilStateInfo_DX12*)(PipelineStateFixed->DepthStencilState))
            ->DepthStencilStateDesc;
  psoDesc.SampleMask = UINT_MAX;
  psoDesc.PrimitiveTopologyType
      = ((jVertexBuffer_DX12*)VertexBufferArray[0])->GetTopologyTypeOnly();
  psoDesc.NumRenderTargets = (uint32_t)RenderPassDX12->GetRTVFormats().size();

  const int32_t NumOfRTVs
      = std::min(static_cast<int32_t>(std::size(psoDesc.RTVFormats)),
                 static_cast<int32_t>(RenderPassDX12->GetRTVFormats().size()));
  for (int32_t i = 0; i < NumOfRTVs; ++i) {
    psoDesc.RTVFormats[i] = RenderPassDX12->GetRTVFormats()[i];
  }
  psoDesc.DSVFormat = RenderPassDX12->GetDSVFormat();

  assert(g_rhi_dx12);
  assert(g_rhi_dx12->Device);

  HRESULT hr = g_rhi_dx12->Device->CreateGraphicsPipelineState(
      &psoDesc, IID_PPV_ARGS(&PipelineState));
  assert(SUCCEEDED(hr));

  if (FAILED(hr)) {
    return nullptr;
  }

  Viewports.resize(PipelineStateFixed->viewports.size());
  for (int32_t i = 0; i < (int32_t)PipelineStateFixed->viewports.size(); ++i) {
    const Viewport& Src = PipelineStateFixed->viewports[i];
    D3D12_VIEWPORT& Dst = Viewports[i];
    Dst.TopLeftX        = Src.X;
    Dst.TopLeftY        = Src.Y;
    Dst.Width           = Src.Width;
    Dst.Height          = Src.Height;
    Dst.MinDepth        = Src.MinDepth;
    Dst.MaxDepth        = Src.MaxDepth;
  }

  Scissors.resize(PipelineStateFixed->scissors.size());
  for (int32_t i = 0; i < (int32_t)PipelineStateFixed->scissors.size(); ++i) {
    const Scissor& Src = PipelineStateFixed->scissors[i];
    D3D12_RECT&    Dst = Scissors[i];
    Dst.left           = Src.Offset.x();
    Dst.right          = Src.Offset.x() + Src.Extent.width();
    Dst.top            = Src.Offset.y();
    Dst.bottom         = Src.Offset.y() + Src.Extent.height();
  }

  size_t hash = GetHash();
  assert(hash);
  if (hash) {
    if (GraphicsShader.VertexShader) {
      Shader::gConnectedPipelineStateHash[GraphicsShader.VertexShader]
          .push_back(hash);
    }
    if (GraphicsShader.GeometryShader) {
      Shader::gConnectedPipelineStateHash[GraphicsShader.GeometryShader]
          .push_back(hash);
    }
    if (GraphicsShader.PixelShader) {
      Shader::gConnectedPipelineStateHash[GraphicsShader.PixelShader].push_back(
          hash);
    }
  }

  return PipelineState.Get();
}

void* jPipelineStateInfo_DX12::CreateComputePipelineState() {
  PipelineState = nullptr;

  D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
  ComPtr<ID3D12RootSignature>       RootSignature
      = jShaderBindingLayout_DX12::CreateRootSignature(
          ShaderBindingLayoutArray);
  psoDesc.pRootSignature = RootSignature.Get();
  if (ComputeShader) {
    auto CS_Compiled
        = (jCompiledShader_DX12*)ComputeShader->GetCompiledShader();
    assert(CS_Compiled);
    psoDesc.CS
        = {.pShaderBytecode = CS_Compiled->ShaderBlob->GetBufferPointer(),
           .BytecodeLength  = CS_Compiled->ShaderBlob->GetBufferSize()};
  }
  assert(g_rhi_dx12);
  assert(g_rhi_dx12->Device);

  HRESULT hr = g_rhi_dx12->Device->CreateComputePipelineState(
      &psoDesc, IID_PPV_ARGS(&PipelineState));
  assert(SUCCEEDED(hr));

  if (FAILED(hr)) {
    return nullptr;
  }

  size_t hash = GetHash();
  assert(hash);
  if (hash) {
    if (ComputeShader) {
      Shader::gConnectedPipelineStateHash[ComputeShader].push_back(hash);
    }
  }

  return PipelineState.Get();
}

void jPipelineStateInfo_DX12::Bind(
    const std::shared_ptr<jRenderFrameContext>& InRenderFrameContext) const {
  auto CommandBuffer_DX12
      = (jCommandBuffer_DX12*)InRenderFrameContext->GetActiveCommandBuffer();
  assert(CommandBuffer_DX12);

  Bind(CommandBuffer_DX12);
}

void jPipelineStateInfo_DX12::Bind(jCommandBuffer_DX12* InCommandList) const {
  assert(InCommandList->CommandList);
  if (PipelineType == jPipelineStateInfo::EPipelineType::Graphics) {
    assert(PipelineState);
    InCommandList->CommandList->SetPipelineState(PipelineState.Get());

    InCommandList->CommandList->RSSetViewports((uint32_t)Viewports.size(),
                                               Viewports.data());
    InCommandList->CommandList->RSSetScissorRects((uint32_t)Scissors.size(),
                                                  Scissors.data());
  } else if (PipelineType == jPipelineStateInfo::EPipelineType::Compute) {
    assert(PipelineState);
    InCommandList->CommandList->SetPipelineState(PipelineState.Get());
  } else if (PipelineType == jPipelineStateInfo::EPipelineType::RayTracing) {
    assert(RaytracingStateObject);
    InCommandList->CommandList->SetPipelineState1(RaytracingStateObject.Get());
  }
}

}  // namespace game_engine