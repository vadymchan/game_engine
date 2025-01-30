#include "gfx/rhi/dx12/pipeline_state_info_dx12.h"

#ifdef GAME_ENGINE_RHI_DX12

#include "gfx/rhi/dx12/buffer_dx12.h"
#include "gfx/rhi/dx12/command_list_dx12.h"
#include "gfx/rhi/dx12/render_pass_dx12.h"
#include "gfx/rhi/dx12/rhi_dx12.h"
#include "gfx/rhi/dx12/shader_binding_layout_dx12.h"
#include "gfx/rhi/dx12/shader_dx12.h"
#include "gfx/rhi/dx12/utils_dx12.h"

#include <algorithm>

namespace game_engine {

void SamplerStateInfoDx12::initialize() {
  D3D12_SAMPLER_DESC samplerDesc = {};

  samplerDesc.AddressU      = g_getDX12TextureAddressMode(m_addressU_);
  samplerDesc.AddressV      = g_getDX12TextureAddressMode(m_addressV_);
  samplerDesc.AddressW      = g_getDX12TextureAddressMode(m_addressW_);
  samplerDesc.MinLOD        = m_minLOD_;
  samplerDesc.MaxLOD        = m_maxLOD_;
  samplerDesc.MipLODBias    = m_mipLODBias_;
  samplerDesc.MaxAnisotropy = (uint32_t)m_maxAnisotropy_;
  if (m_maxAnisotropy_ > 1) {
    samplerDesc.Filter = D3D12_FILTER_ANISOTROPIC;
  } else {
    samplerDesc.Filter = g_getDX12TextureFilter(
        m_minification_, m_magnification_, m_isEnableComparisonMode_);
  }
  samplerDesc.ComparisonFunc = g_getDX12CompareOp(m_comparisonFunc_);

  assert(sizeof(samplerDesc.BorderColor) == sizeof(m_borderColor_));
  memcpy(samplerDesc.BorderColor, &m_borderColor_, sizeof(m_borderColor_));

  m_samplerSRV_ = g_rhiDx12->m_samplerDescriptorHeaps_.alloc();

  assert(g_rhiDx12);
  assert(g_rhiDx12->m_device_);
  g_rhiDx12->m_device_->CreateSampler(&samplerDesc, m_samplerSRV_.m_cpuHandle_);

  m_resourceName_ = Name(toString().c_str());
}

void RasterizationStateInfoDx12::initialize() {
  m_rasterizeDesc_.FillMode              = g_getDX12PolygonMode(m_polygonMode_);
  m_rasterizeDesc_.CullMode              = g_getDX12CullMode(m_cullMode_);
  m_rasterizeDesc_.FrontCounterClockwise = m_frontFace_ == EFrontFace::CCW;
  m_rasterizeDesc_.DepthBias
      = (int32_t)(m_depthBiasEnable_ ? m_depthBiasConstantFactor_ : 0);
  m_rasterizeDesc_.DepthBiasClamp       = m_depthBiasClamp_;
  m_rasterizeDesc_.SlopeScaledDepthBias = m_depthBiasSlopeFactor_;
  m_rasterizeDesc_.DepthClipEnable      = m_depthClampEnable_;
  m_rasterizeDesc_.MultisampleEnable    = (int32_t)m_sampleCount_ > 1;

  // RasterizeDesc.AntialiasedLineEnable;
  // RasterizeDesc.ForcedSampleCount;
  // RasterizeDesc.ConservativeRaster;

  m_multiSampleDesc_.Count = (int32_t)m_sampleCount_;
}

void StencilOpStateInfoDx12::initialize() {
  m_stencilOpStateDesc_.StencilDepthFailOp = g_getDX12StencilOp(m_depthFailOp_);
  m_stencilOpStateDesc_.StencilFailOp      = g_getDX12StencilOp(m_failOp_);
  m_stencilOpStateDesc_.StencilFunc        = g_getDX12CompareOp(m_compareOp_);
  m_stencilOpStateDesc_.StencilPassOp      = g_getDX12StencilOp(m_passOp_);
}

void DepthStencilStateInfoDx12::initialize() {
  m_depthStencilStateDesc_.DepthEnable    = m_depthTestEnable_;
  m_depthStencilStateDesc_.DepthWriteMask = m_depthWriteEnable_
                                              ? D3D12_DEPTH_WRITE_MASK_ALL
                                              : D3D12_DEPTH_WRITE_MASK_ZERO;
  m_depthStencilStateDesc_.DepthFunc = g_getDX12CompareOp(m_depthCompareOp_);
  m_depthStencilStateDesc_.StencilEnable = m_stencilTestEnable_;
  // DepthStencilStateDesc.StencilReadMask;
  // DepthStencilStateDesc.StencilWriteMask;
  if (m_front_) {
    m_depthStencilStateDesc_.FrontFace
        = ((StencilOpStateInfoDx12*)m_front_)->m_stencilOpStateDesc_;
  }

  if (m_back_) {
    m_depthStencilStateDesc_.BackFace
        = ((StencilOpStateInfoDx12*)m_back_)->m_stencilOpStateDesc_;
  }

  // MinDepthBounds;
  // MaxDepthBounds;
}

void BlendingStateInfoDx12::initialize() {
  m_blendDesc_.BlendEnable           = m_blendEnable_;
  m_blendDesc_.SrcBlend              = g_getDX12BlendFactor(m_src_);
  m_blendDesc_.DestBlend             = g_getDX12BlendFactor(m_dest_);
  m_blendDesc_.BlendOp               = g_getDX12BlendOp(m_blendOp_);
  m_blendDesc_.SrcBlendAlpha         = g_getDX12BlendFactor(m_srcAlpha_);
  m_blendDesc_.DestBlendAlpha        = g_getDX12BlendFactor(m_destAlpha_);
  m_blendDesc_.BlendOpAlpha          = g_getDX12BlendOp(m_alphaBlendOp_);
  m_blendDesc_.RenderTargetWriteMask = g_getDX12ColorMask(m_colorWriteMask_);

  // BlendDesc.LogicOpEnable;
  // BlendDesc.LogicOp;
}

void SamplerStateInfoDx12::release() {
  m_samplerSRV_.free();
}

void PipelineStateInfoDx12::release() {
}

void PipelineStateInfoDx12::initialize() {
  if (m_pipelineType_ == EPipelineType::Graphics) {
    createGraphicsPipelineState();
  } else if (m_pipelineType_ == EPipelineType::Compute) {
    createComputePipelineState();
  } else if (m_pipelineType_ == EPipelineType::RayTracing) {
    // createRaytracingPipelineState();
  } else {
    assert(0);
  }
}

void* PipelineStateInfoDx12::createGraphicsPipelineState() {
  m_pipelineState_ = nullptr;

  D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
  assert(m_vertexBufferArray_.m_numOfData_ > 0);

  // Input layout setup
  std::vector<D3D12_INPUT_ELEMENT_DESC> inputElementDescs;
  VertexBufferDx12::s_createVertexInputState(inputElementDescs,
                                             m_vertexBufferArray_);
  psoDesc.InputLayout.pInputElementDescs = inputElementDescs.data();
  psoDesc.InputLayout.NumElements = static_cast<UINT>(inputElementDescs.size());

  // Root signature
  ComPtr<ID3D12RootSignature> RootSignature
      = ShaderBindingLayoutDx12::s_createRootSignature(
          m_shaderBindingLayoutArray_);
  psoDesc.pRootSignature = RootSignature.Get();

  // Shaders
  if (kGraphicsShader.m_vertexShader_) {
    auto VS_Compiled = std::static_pointer_cast<CompiledShaderDx12>(
        kGraphicsShader.m_vertexShader_->getCompiledShader());
    assert(VS_Compiled);
    psoDesc.VS
        = {.pShaderBytecode = VS_Compiled->m_shaderBlob_->GetBufferPointer(),
           .BytecodeLength  = VS_Compiled->m_shaderBlob_->GetBufferSize()};
  }
  if (kGraphicsShader.m_geometryShader_) {
    auto GS_Compiled = std::static_pointer_cast<CompiledShaderDx12>(
        kGraphicsShader.m_geometryShader_->getCompiledShader());
    assert(GS_Compiled);
    psoDesc.GS
        = {.pShaderBytecode = GS_Compiled->m_shaderBlob_->GetBufferPointer(),
           .BytecodeLength  = GS_Compiled->m_shaderBlob_->GetBufferSize()};
  }
  if (kGraphicsShader.m_pixelShader_) {
    auto PS_Compiled = std::static_pointer_cast<CompiledShaderDx12>(
        kGraphicsShader.m_pixelShader_->getCompiledShader());
    assert(PS_Compiled);
    psoDesc.PS
        = {.pShaderBytecode = PS_Compiled->m_shaderBlob_->GetBufferPointer(),
           .BytecodeLength  = PS_Compiled->m_shaderBlob_->GetBufferSize()};
  }

  // Rasterizer / SampleDesc
  psoDesc.RasterizerState = static_cast<RasterizationStateInfoDx12*>(
                                kPipelineStateFixed->m_rasterizationState_)
                                ->m_rasterizeDesc_;
  psoDesc.SampleDesc.Count = static_cast<RasterizationStateInfoDx12*>(
                                 kPipelineStateFixed->m_rasterizationState_)
                                 ->m_multiSampleDesc_.Count;

  // Blending
  auto renderPassDx12 = static_cast<const RenderPassDx12*>(kRenderPass);
  for (int32_t i = 0;
       i < static_cast<int32_t>(renderPassDx12->getRTVFormats().size());
       ++i) {
    psoDesc.BlendState.RenderTarget[i]
        = static_cast<BlendingStateInfoDx12*>(
              kPipelineStateFixed->m_blendingState_)
              ->m_blendDesc_;
  }

  // Depth/Stencil handling 
  auto* depthStencilInfo = static_cast<DepthStencilStateInfoDx12*>(
      kPipelineStateFixed->m_depthStencilState_);


  DXGI_FORMAT dsvFormat = renderPassDx12->getDSVFormat();
  bool        hasDepth  = (dsvFormat != DXGI_FORMAT_UNKNOWN);

  if (hasDepth) {
    psoDesc.DepthStencilState = depthStencilInfo->m_depthStencilStateDesc_;
    psoDesc.DSVFormat         = dsvFormat;
  } else {
    // Fully disable depth/stencil
    D3D12_DEPTH_STENCIL_DESC disabledDepth = {};
    disabledDepth.DepthEnable              = FALSE;
    disabledDepth.DepthWriteMask           = D3D12_DEPTH_WRITE_MASK_ZERO;
    disabledDepth.DepthFunc                = D3D12_COMPARISON_FUNC_ALWAYS;
    disabledDepth.StencilEnable            = FALSE;

    psoDesc.DepthStencilState = disabledDepth;
    psoDesc.DSVFormat         = DXGI_FORMAT_UNKNOWN;
  }

  // Other pipeline parameters
  psoDesc.SampleMask = UINT_MAX;
  psoDesc.PrimitiveTopologyType
      = ((VertexBufferDx12*)m_vertexBufferArray_[0])->getTopologyTypeOnly();
  psoDesc.NumRenderTargets
      = static_cast<UINT>(renderPassDx12->getRTVFormats().size());

  const int32_t numOfRTVs = std::min<int32_t>(
      static_cast<int32_t>(std::size(psoDesc.RTVFormats)),
      static_cast<int32_t>(renderPassDx12->getRTVFormats().size()));
  for (int32_t i = 0; i < numOfRTVs; ++i) {
    psoDesc.RTVFormats[i] = renderPassDx12->getRTVFormats()[i];
  }

  // Create the pipeline
  assert(g_rhiDx12);
  assert(g_rhiDx12->m_device_);
  HRESULT hr = g_rhiDx12->m_device_->CreateGraphicsPipelineState(
      &psoDesc, IID_PPV_ARGS(&m_pipelineState_));
  assert(SUCCEEDED(hr));
  if (FAILED(hr)) {
    return nullptr;
  }

  // Copy viewports/scissors
  m_viewports_.resize(kPipelineStateFixed->m_viewports_.size());
  for (int32_t i = 0;
       i < static_cast<int32_t>(kPipelineStateFixed->m_viewports_.size());
       ++i) {
    const Viewport& src = kPipelineStateFixed->m_viewports_[i];
    D3D12_VIEWPORT& dst = m_viewports_[i];
    dst.TopLeftX        = src.m_x_;
    dst.TopLeftY        = src.m_y_;
    dst.Width           = src.m_width_;
    dst.Height          = src.m_height_;
    dst.MinDepth        = src.m_minDepth_;
    dst.MaxDepth        = src.m_maxDepth_;
  }

  m_scissors_.resize(kPipelineStateFixed->m_scissors_.size());
  for (int32_t i = 0;
       i < static_cast<int32_t>(kPipelineStateFixed->m_scissors_.size());
       ++i) {
    const Scissor& src = kPipelineStateFixed->m_scissors_[i];
    D3D12_RECT&    dst = m_scissors_[i];
    dst.left           = src.m_offset_.x();
    dst.right          = src.m_offset_.x() + src.m_extent_.width();
    dst.top            = src.m_offset_.y();
    dst.bottom         = src.m_offset_.y() + src.m_extent_.height();
  }

  return m_pipelineState_.Get();
}

void* PipelineStateInfoDx12::createComputePipelineState() {
  m_pipelineState_ = nullptr;

  D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
  ComPtr<ID3D12RootSignature>       RootSignature
      = ShaderBindingLayoutDx12::s_createRootSignature(
          m_shaderBindingLayoutArray_);
  psoDesc.pRootSignature = RootSignature.Get();
  if (computeShader) {
    auto CS_Compiled = std::static_pointer_cast<CompiledShaderDx12>(
        computeShader->getCompiledShader());
    assert(CS_Compiled);
    psoDesc.CS
        = {.pShaderBytecode = CS_Compiled->m_shaderBlob_->GetBufferPointer(),
           .BytecodeLength  = CS_Compiled->m_shaderBlob_->GetBufferSize()};
  }
  assert(g_rhiDx12);
  assert(g_rhiDx12->m_device_);

  HRESULT hr = g_rhiDx12->m_device_->CreateComputePipelineState(
      &psoDesc, IID_PPV_ARGS(&m_pipelineState_));
  assert(SUCCEEDED(hr));

  if (FAILED(hr)) {
    return nullptr;
  }

  // size_t hash = getHash();
  // assert(hash);
  // if (hash) {
  //   if (computeShader) {
  //     Shader::s_connectedPipelineStateHash[computeShader].push_back(hash);
  //   }
  // }

  return m_pipelineState_.Get();
}

void PipelineStateInfoDx12::bind(
    const std::shared_ptr<CommandBuffer>& commandBuffer) const {
  auto commandBufferDx12
      = std::static_pointer_cast<CommandBufferDx12>(commandBuffer);
  assert(commandBufferDx12);
  assert(commandBufferDx12->m_commandList_);
  if (m_pipelineType_ == PipelineStateInfo::EPipelineType::Graphics) {
    assert(m_pipelineState_);
    commandBufferDx12->m_commandList_->SetPipelineState(m_pipelineState_.Get());

    commandBufferDx12->m_commandList_->RSSetViewports(
        (uint32_t)m_viewports_.size(), m_viewports_.data());
    commandBufferDx12->m_commandList_->RSSetScissorRects(
        (uint32_t)m_scissors_.size(), m_scissors_.data());
  } else if (m_pipelineType_ == PipelineStateInfo::EPipelineType::Compute) {
    assert(m_pipelineState_);
    commandBufferDx12->m_commandList_->SetPipelineState(m_pipelineState_.Get());
  } else if (m_pipelineType_ == PipelineStateInfo::EPipelineType::RayTracing) {
    // TODO: currently not implemented
    assert(0);
  }
}

}  // namespace game_engine

#endif  // GAME_ENGINE_RHI_DX12
