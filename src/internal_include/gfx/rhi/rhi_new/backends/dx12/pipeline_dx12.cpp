#include "gfx/rhi/rhi_new/backends/dx12/pipeline_dx12.h"

#ifdef GAME_ENGINE_RHI_DX12

#include "gfx/rhi/rhi_new/backends/dx12/descriptor_dx12.h"
#include "gfx/rhi/rhi_new/backends/dx12/device_dx12.h"
#include "gfx/rhi/rhi_new/backends/dx12/render_pass_dx12.h"
#include "gfx/rhi/rhi_new/backends/dx12/rhi_enums_dx12.h"
#include "gfx/rhi/rhi_new/backends/dx12/shader_dx12.h"
#include "utils/logger/global_logger.h"

#include <d3dcompiler.h>

namespace game_engine {
namespace gfx {
namespace rhi {

GraphicsPipelineDx12::GraphicsPipelineDx12(const GraphicsPipelineDesc& desc, DeviceDx12* device)
    : GraphicsPipeline(desc)
    , m_device_(device) {
  if (!initialize_()) {
    GlobalLogger::Log(LogLevel::Error, "Failed to initialize DirectX 12 graphics pipeline");
  }
}

bool GraphicsPipelineDx12::initialize_() {
  // First create the root signature
  if (!createRootSignature_()) {
    GlobalLogger::Log(LogLevel::Error, "Failed to create root signature for DX12 pipeline");
    return false;
  }

  // Then create the pipeline state object
  if (!createPipelineState_()) {
    GlobalLogger::Log(LogLevel::Error, "Failed to create pipeline state object for DX12 pipeline");
    return false;
  }

  return true;
}

bool GraphicsPipelineDx12::createRootSignature_() {
  // Calculate the number of parameters needed
  std::vector<CD3DX12_ROOT_PARAMETER>   rootParameters;
  std::vector<CD3DX12_DESCRIPTOR_RANGE> descriptorRanges;

  // Clear owned descriptor set layouts
  m_ownedDescriptorSetLayouts_.clear();

  // For each descriptor set layout
  uint32_t setIndex = 0;
  for (const auto& layoutDesc : m_desc_.setLayouts) {
    // Create descriptor set layout object and take ownership
    auto layout = m_device_->createDescriptorSetLayout(layoutDesc);
    if (!layout) {
      GlobalLogger::Log(LogLevel::Error, "Failed to create descriptor set layout");
      return false;
    }

    DescriptorSetLayoutDx12* layoutDx12 = dynamic_cast<DescriptorSetLayoutDx12*>(layout.get());
    if (!layoutDx12) {
      GlobalLogger::Log(LogLevel::Error, "Invalid descriptor set layout type for DX12 pipeline");
      return false;
    }

    // For each binding in this layout
    for (const auto& binding : layoutDesc.bindings) {
      // Create a descriptor range for this binding
      CD3DX12_DESCRIPTOR_RANGE range;

      // Convert binding type to D3D12_DESCRIPTOR_RANGE_TYPE
      D3D12_DESCRIPTOR_RANGE_TYPE rangeType = g_getShaderBindingTypeDx12(binding.type);

      // Initialize the range
      range.Init(rangeType,                // Range type
                 binding.descriptorCount,  // Number of descriptors
                 binding.binding,          // Base shader register
                 setIndex                  // Register space (use set index)
      );

      // Store the range
      descriptorRanges.push_back(range);

      // Create a root parameter for this range
      CD3DX12_ROOT_PARAMETER param;
      param.InitAsDescriptorTable(1, &descriptorRanges.back());

      // Store the parameter
      rootParameters.push_back(param);

      // Map the descriptor set and binding to the parameter index
      m_descriptorTableMap[setIndex] = static_cast<uint32_t>(rootParameters.size() - 1);
    }

    // Store the layout for lifetime management
    m_ownedDescriptorSetLayouts_.push_back(std::move(layout));

    setIndex++;
  }

  // Create a static sampler array if needed (not implementing here for simplicity)

  // Create the root signature description
  CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
  rootSignatureDesc.Init(static_cast<UINT>(rootParameters.size()),
                         rootParameters.empty() ? nullptr : rootParameters.data(),
                         0,
                         nullptr,  // Static samplers
                         D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

  // Serialize the root signature
  ComPtr<ID3DBlob> signature;
  ComPtr<ID3DBlob> error;
  HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);

  if (FAILED(hr)) {
    std::string errorMsg = "Failed to serialize root signature: ";
    if (error) {
      errorMsg += static_cast<char*>(error->GetBufferPointer());
    }
    GlobalLogger::Log(LogLevel::Error, errorMsg);
    return false;
  }

  // Create the root signature
  hr = m_device_->getDevice()->CreateRootSignature(0,  // Node mask
                                                   signature->GetBufferPointer(),
                                                   signature->GetBufferSize(),
                                                   IID_PPV_ARGS(&m_rootSignature_));

  if (FAILED(hr)) {
    GlobalLogger::Log(LogLevel::Error, "Failed to create root signature");
    return false;
  }

  return true;
}

bool GraphicsPipelineDx12::createPipelineState_() {
  // Collect shader bytecode from our shader descriptors
  ComPtr<ID3DBlob> vertexShader;
  ComPtr<ID3DBlob> pixelShader;
  ComPtr<ID3DBlob> domainShader;
  ComPtr<ID3DBlob> hullShader;
  ComPtr<ID3DBlob> geometryShader;

  if (!collectShaders_(vertexShader, pixelShader, domainShader, hullShader, geometryShader)) {
    GlobalLogger::Log(LogLevel::Error, "Failed to collect shaders for pipeline");
    return false;
  }

  // Create input layout elements
  std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout;

  for (const auto& attribute : m_desc_.vertexAttributes) {
    D3D12_INPUT_ELEMENT_DESC element = {};

    // Use format as the semantic name for simplicity
    // In a real implementation, you'd probably want a proper semantic name system
    element.SemanticName      = "SEMANTIC";  // Placeholder, needs to be adjusted based on your semantics system
    element.SemanticIndex     = attribute.location;
    element.Format            = g_getTextureFormatDx12(attribute.format);
    element.InputSlot         = attribute.binding;
    element.AlignedByteOffset = attribute.offset;

    // Find the matching binding
    for (const auto& binding : m_desc_.vertexBindings) {
      if (binding.binding == attribute.binding) {
        element.InputSlotClass = g_getVertexInputRateDx12(binding.inputRate);

        // For per-instance, this is the number of instances to draw using the same instance data
        element.InstanceDataStepRate = (binding.inputRate == VertexInputRate::Instance) ? 1 : 0;
        break;
      }
    }

    inputLayout.push_back(element);
  }

  // Setup the graphics pipeline state
  D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};

  // Setup the root signature
  psoDesc.pRootSignature = m_rootSignature_.Get();

  // Setup shader stages
  if (vertexShader) {
    psoDesc.VS.pShaderBytecode = vertexShader->GetBufferPointer();
    psoDesc.VS.BytecodeLength  = vertexShader->GetBufferSize();
  }

  if (pixelShader) {
    psoDesc.PS.pShaderBytecode = pixelShader->GetBufferPointer();
    psoDesc.PS.BytecodeLength  = pixelShader->GetBufferSize();
  }

  if (domainShader) {
    psoDesc.DS.pShaderBytecode = domainShader->GetBufferPointer();
    psoDesc.DS.BytecodeLength  = domainShader->GetBufferSize();
  }

  if (hullShader) {
    psoDesc.HS.pShaderBytecode = hullShader->GetBufferPointer();
    psoDesc.HS.BytecodeLength  = hullShader->GetBufferSize();
  }

  if (geometryShader) {
    psoDesc.GS.pShaderBytecode = geometryShader->GetBufferPointer();
    psoDesc.GS.BytecodeLength  = geometryShader->GetBufferSize();
  }

  // Setup stream output (not used in this example)
  psoDesc.StreamOutput.pSODeclaration   = nullptr;
  psoDesc.StreamOutput.NumEntries       = 0;
  psoDesc.StreamOutput.pBufferStrides   = nullptr;
  psoDesc.StreamOutput.NumStrides       = 0;
  psoDesc.StreamOutput.RasterizedStream = 0;

  // Setup the blend state
  psoDesc.BlendState.AlphaToCoverageEnable  = m_desc_.multisample.alphaToCoverageEnable;
  psoDesc.BlendState.IndependentBlendEnable = TRUE;  // Allow different blend states for each render target

  // Setup blend state for each render target
  for (uint32_t i = 0; i < 8; i++) {
    if (i < m_desc_.colorBlend.attachments.size()) {
      const auto& attachment = m_desc_.colorBlend.attachments[i];

      psoDesc.BlendState.RenderTarget[i].BlendEnable   = attachment.blendEnable;
      psoDesc.BlendState.RenderTarget[i].LogicOpEnable = m_desc_.colorBlend.logicOpEnable;

      // Set blend operations
      psoDesc.BlendState.RenderTarget[i].SrcBlend  = g_getBlendFactorDx12(attachment.srcColorBlendFactor);
      psoDesc.BlendState.RenderTarget[i].DestBlend = g_getBlendFactorDx12(attachment.dstColorBlendFactor);
      psoDesc.BlendState.RenderTarget[i].BlendOp   = g_getBlendOpDx12(attachment.colorBlendOp);

      psoDesc.BlendState.RenderTarget[i].SrcBlendAlpha  = g_getBlendFactorDx12(attachment.srcAlphaBlendFactor);
      psoDesc.BlendState.RenderTarget[i].DestBlendAlpha = g_getBlendFactorDx12(attachment.dstAlphaBlendFactor);
      psoDesc.BlendState.RenderTarget[i].BlendOpAlpha   = g_getBlendOpDx12(attachment.alphaBlendOp);

      // Set logic operation
      psoDesc.BlendState.RenderTarget[i].LogicOp = static_cast<D3D12_LOGIC_OP>(m_desc_.colorBlend.logicOp);

      // Set write mask
      psoDesc.BlendState.RenderTarget[i].RenderTargetWriteMask = g_getColorMaskDx12(attachment.colorWriteMask);
    } else {
      // Set default blend state for unused render targets
      psoDesc.BlendState.RenderTarget[i].BlendEnable           = FALSE;
      psoDesc.BlendState.RenderTarget[i].LogicOpEnable         = FALSE;
      psoDesc.BlendState.RenderTarget[i].SrcBlend              = D3D12_BLEND_ONE;
      psoDesc.BlendState.RenderTarget[i].DestBlend             = D3D12_BLEND_ZERO;
      psoDesc.BlendState.RenderTarget[i].BlendOp               = D3D12_BLEND_OP_ADD;
      psoDesc.BlendState.RenderTarget[i].SrcBlendAlpha         = D3D12_BLEND_ONE;
      psoDesc.BlendState.RenderTarget[i].DestBlendAlpha        = D3D12_BLEND_ZERO;
      psoDesc.BlendState.RenderTarget[i].BlendOpAlpha          = D3D12_BLEND_OP_ADD;
      psoDesc.BlendState.RenderTarget[i].LogicOp               = D3D12_LOGIC_OP_NOOP;
      psoDesc.BlendState.RenderTarget[i].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    }
  }

  // Copy blend constants
  for (int i = 0; i < 4; i++) {
    m_blendFactors_[i] = m_desc_.colorBlend.blendConstants[i];
  }

  // Setup the sample mask
  psoDesc.SampleMask = m_desc_.multisample.sampleMask;

  // Setup the rasterizer state
  psoDesc.RasterizerState.FillMode              = g_getPolygonModeDx12(m_desc_.rasterization.polygonMode);
  psoDesc.RasterizerState.CullMode              = g_getCullModeDx12(m_desc_.rasterization.cullMode);
  psoDesc.RasterizerState.FrontCounterClockwise = (m_desc_.rasterization.frontFace == FrontFace::Ccw) ? TRUE : FALSE;
  psoDesc.RasterizerState.DepthBias             = static_cast<INT>(m_desc_.rasterization.depthBiasConstantFactor);
  psoDesc.RasterizerState.DepthBiasClamp        = m_desc_.rasterization.depthBiasClamp;
  psoDesc.RasterizerState.SlopeScaledDepthBias  = m_desc_.rasterization.depthBiasSlopeFactor;
  psoDesc.RasterizerState.DepthClipEnable = m_desc_.rasterization.depthClampEnable ? FALSE : TRUE;  // Inverted logic
  psoDesc.RasterizerState.MultisampleEnable
      = (m_desc_.multisample.rasterizationSamples != MSAASamples::Count1) ? TRUE : FALSE;
  psoDesc.RasterizerState.AntialiasedLineEnable = FALSE;  // Not used in this example
  psoDesc.RasterizerState.ForcedSampleCount     = 0;      // Use the sample count from the render target
  psoDesc.RasterizerState.ConservativeRaster    = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

  // Setup the depth-stencil state
  psoDesc.DepthStencilState.DepthEnable = m_desc_.depthStencil.depthTestEnable;
  psoDesc.DepthStencilState.DepthWriteMask
      = m_desc_.depthStencil.depthWriteEnable ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
  psoDesc.DepthStencilState.DepthFunc        = g_getCompareOpDx12(m_desc_.depthStencil.depthCompareOp);
  psoDesc.DepthStencilState.StencilEnable    = m_desc_.depthStencil.stencilTestEnable;
  psoDesc.DepthStencilState.StencilReadMask  = static_cast<UINT8>(m_desc_.depthStencil.front.compareMask);
  psoDesc.DepthStencilState.StencilWriteMask = static_cast<UINT8>(m_desc_.depthStencil.front.writeMask);

  // Setup front face stencil operations
  psoDesc.DepthStencilState.FrontFace.StencilFailOp      = g_getStencilOpDx12(m_desc_.depthStencil.front.failOp);
  psoDesc.DepthStencilState.FrontFace.StencilDepthFailOp = g_getStencilOpDx12(m_desc_.depthStencil.front.depthFailOp);
  psoDesc.DepthStencilState.FrontFace.StencilPassOp      = g_getStencilOpDx12(m_desc_.depthStencil.front.passOp);
  psoDesc.DepthStencilState.FrontFace.StencilFunc        = g_getCompareOpDx12(m_desc_.depthStencil.front.compareOp);

  // Setup back face stencil operations
  psoDesc.DepthStencilState.BackFace.StencilFailOp      = g_getStencilOpDx12(m_desc_.depthStencil.back.failOp);
  psoDesc.DepthStencilState.BackFace.StencilDepthFailOp = g_getStencilOpDx12(m_desc_.depthStencil.back.depthFailOp);
  psoDesc.DepthStencilState.BackFace.StencilPassOp      = g_getStencilOpDx12(m_desc_.depthStencil.back.passOp);
  psoDesc.DepthStencilState.BackFace.StencilFunc        = g_getCompareOpDx12(m_desc_.depthStencil.back.compareOp);

  // Setup input layout
  psoDesc.InputLayout.pInputElementDescs = inputLayout.data();
  psoDesc.InputLayout.NumElements        = static_cast<UINT>(inputLayout.size());

  // Setup primitive topology
  psoDesc.PrimitiveTopologyType = g_getPrimitiveTopologyTypeOnlyDx12(m_desc_.inputAssembly.topology);

  // Setup render target formats
  RenderPassDx12* renderPassDx12 = dynamic_cast<RenderPassDx12*>(m_desc_.renderPass);
  if (!renderPassDx12) {
    GlobalLogger::Log(LogLevel::Error, "Invalid render pass for DX12 pipeline");
    return false;
  }

  const auto& renderTargetFormats = renderPassDx12->getColorFormats();

  psoDesc.NumRenderTargets = static_cast<UINT>(renderTargetFormats.size());
  for (UINT i = 0; i < psoDesc.NumRenderTargets; i++) {
    psoDesc.RTVFormats[i] = renderTargetFormats[i];
  }

  // Setup depth-stencil format
  psoDesc.DSVFormat = renderPassDx12->getDepthStencilFormat();

  // Setup sample description
  switch (m_desc_.multisample.rasterizationSamples) {
    case MSAASamples::Count1:
      psoDesc.SampleDesc.Count = 1;
      break;
    case MSAASamples::Count2:
      psoDesc.SampleDesc.Count = 2;
      break;
    case MSAASamples::Count4:
      psoDesc.SampleDesc.Count = 4;
      break;
    case MSAASamples::Count8:
      psoDesc.SampleDesc.Count = 8;
      break;
    case MSAASamples::Count16:
      psoDesc.SampleDesc.Count = 16;
      break;
    default:
      psoDesc.SampleDesc.Count = 1;
      break;
  }
  psoDesc.SampleDesc.Quality = 0;

  // Setup node mask and caching
  psoDesc.NodeMask                        = 0;
  psoDesc.CachedPSO.pCachedBlob           = nullptr;
  psoDesc.CachedPSO.CachedBlobSizeInBytes = 0;

  // Set the pipeline flag (graphics pipeline)
  psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

  // Create the pipeline state object
  HRESULT hr = m_device_->getDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState_));

  if (FAILED(hr)) {
    GlobalLogger::Log(LogLevel::Error, "Failed to create graphics pipeline state object");
    return false;
  }

  return true;
}

bool GraphicsPipelineDx12::collectShaders_(ComPtr<ID3DBlob>& vertexShader,
                                           ComPtr<ID3DBlob>& pixelShader,
                                           ComPtr<ID3DBlob>& domainShader,
                                           ComPtr<ID3DBlob>& hullShader,
                                           ComPtr<ID3DBlob>& geometryShader) {
  for (Shader* shader : m_desc_.shaders) {
    if (!shader) {
      GlobalLogger::Log(LogLevel::Error, "Null shader pointer in pipeline description");
      return false;
    }

    ShaderDx12* shaderDx12 = dynamic_cast<ShaderDx12*>(shader);
    if (!shaderDx12) {
      GlobalLogger::Log(LogLevel::Error, "Invalid shader type for DX12 pipeline");
      return false;
    }

    switch (shaderDx12->getStage()) {
      case ShaderStageFlag::Vertex:
        vertexShader = shaderDx12->getShaderBlob();
        break;
      case ShaderStageFlag::Fragment:
        pixelShader = shaderDx12->getShaderBlob();
        break;
      case ShaderStageFlag::TessellationEvaluation:
        domainShader = shaderDx12->getShaderBlob();
        break;
      case ShaderStageFlag::TessellationControl:
        hullShader = shaderDx12->getShaderBlob();
        break;
      case ShaderStageFlag::Geometry:
        geometryShader = shaderDx12->getShaderBlob();
        break;
      default:
        GlobalLogger::Log(LogLevel::Warning, "Unsupported shader stage for DX12 pipeline");
        break;
    }
  }

  return true;
}

}  // namespace rhi
}  // namespace gfx
}  // namespace game_engine

#endif  // GAME_ENGINE_RHI_DX12