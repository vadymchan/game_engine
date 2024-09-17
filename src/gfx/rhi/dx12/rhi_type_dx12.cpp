
#include "gfx/rhi/dx12/rhi_type_dx12.h"

#ifdef GAME_ENGINE_RHI_DX12

#include "gfx/rhi/dx12/rhi_dx12.h"


namespace game_engine {
// clang-format off


static const std::unordered_map<ETextureFormat, DXGI_FORMAT> textureFormatMappingToDXGI = {
    {ETextureFormat::RGB8,       DXGI_FORMAT_R8G8B8A8_UNORM},          // not support rgb8 -> rgba8
    {ETextureFormat::RGB32F,     DXGI_FORMAT_R32G32B32A32_FLOAT},      // not support for UAV rgb32 -> rgba32
    {ETextureFormat::RGB16F,     DXGI_FORMAT_R16G16B16A16_FLOAT},      // not support rgb16 -> rgba16
    {ETextureFormat::R11G11B10F, DXGI_FORMAT_R11G11B10_FLOAT},

    {ETextureFormat::RGBA8,      DXGI_FORMAT_R8G8B8A8_UNORM},
    {ETextureFormat::RGBA16F,    DXGI_FORMAT_R16G16B16A16_FLOAT},
    {ETextureFormat::RGBA32F,    DXGI_FORMAT_R32G32B32A32_FLOAT},
    {ETextureFormat::RGBA8SI,    DXGI_FORMAT_R8G8B8A8_SINT},
    {ETextureFormat::RGBA8UI,    DXGI_FORMAT_R8G8B8A8_UINT},

    {ETextureFormat::BGRA8,      DXGI_FORMAT_B8G8R8A8_UNORM},

    {ETextureFormat::R8,         DXGI_FORMAT_R8_UNORM},
    {ETextureFormat::R16F,       DXGI_FORMAT_R16_FLOAT},
    {ETextureFormat::R32F,       DXGI_FORMAT_R32_FLOAT},
    {ETextureFormat::R8UI,       DXGI_FORMAT_R8_UINT},
    {ETextureFormat::R32UI,      DXGI_FORMAT_R32_UINT},

    {ETextureFormat::RG8,        DXGI_FORMAT_R8G8_UNORM},
    {ETextureFormat::RG16F,      DXGI_FORMAT_R16G16_FLOAT},
    {ETextureFormat::RG32F,      DXGI_FORMAT_R32G32_FLOAT},

    {ETextureFormat::D16,        DXGI_FORMAT_D16_UNORM},
    {ETextureFormat::D16_S8,     DXGI_FORMAT_D24_UNORM_S8_UINT},       // not support d16_s8 -> d24_s8
    {ETextureFormat::D24,        DXGI_FORMAT_D24_UNORM_S8_UINT},
    {ETextureFormat::D24_S8,     DXGI_FORMAT_D24_UNORM_S8_UINT},
    {ETextureFormat::D32,        DXGI_FORMAT_D32_FLOAT},
    {ETextureFormat::D32_S8,     DXGI_FORMAT_D32_FLOAT},               // Use DXGI_FORMAT_D32_FLOAT for this case

    {ETextureFormat::BC1_UNORM,  DXGI_FORMAT_BC1_UNORM},
    {ETextureFormat::BC2_UNORM,  DXGI_FORMAT_BC2_UNORM},
    {ETextureFormat::BC3_UNORM,  DXGI_FORMAT_BC3_UNORM},
    {ETextureFormat::BC4_UNORM,  DXGI_FORMAT_BC4_UNORM},
    {ETextureFormat::BC4_SNORM,  DXGI_FORMAT_BC4_SNORM},
    {ETextureFormat::BC5_UNORM,  DXGI_FORMAT_BC5_UNORM},
    {ETextureFormat::BC5_SNORM,  DXGI_FORMAT_BC5_SNORM},
    {ETextureFormat::BC6H_UF16,  DXGI_FORMAT_BC6H_UF16},
    {ETextureFormat::BC6H_SF16,  DXGI_FORMAT_BC6H_SF16},
    {ETextureFormat::BC7_UNORM,  DXGI_FORMAT_BC7_UNORM}
};


static const std::unordered_map<DXGI_FORMAT, ETextureFormat> textureFormatMappingToGeneric = reverseMap(textureFormatMappingToDXGI);


DXGI_FORMAT g_getDX12TextureFormat(ETextureFormat textureFormat) {
    return getEnumMapping(textureFormatMappingToDXGI, textureFormat, DXGI_FORMAT_UNKNOWN);
}

ETextureFormat g_getDX12TextureFormat(DXGI_FORMAT formatType) {
    return getEnumMapping(textureFormatMappingToGeneric, formatType, ETextureFormat::MAX);
}

// ------------------- DESCRIPTOR HEAP TYPE -------------------

static const std::unordered_map<EDescriptorHeapTypeDX12, D3D12_DESCRIPTOR_HEAP_TYPE> heapTypeMapping = {
    {EDescriptorHeapTypeDX12::CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV},
    {EDescriptorHeapTypeDX12::SAMPLER,     D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER},
    {EDescriptorHeapTypeDX12::RTV,         D3D12_DESCRIPTOR_HEAP_TYPE_RTV},
    {EDescriptorHeapTypeDX12::DSV,         D3D12_DESCRIPTOR_HEAP_TYPE_DSV}
};

static const std::unordered_map<D3D12_DESCRIPTOR_HEAP_TYPE, EDescriptorHeapTypeDX12> heapTypeReverseMapping = reverseMap(heapTypeMapping);

D3D12_DESCRIPTOR_HEAP_TYPE g_getDX12DescriptorHeapType(EDescriptorHeapTypeDX12 heapType) {
    return getEnumMapping(heapTypeMapping, heapType, D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES);
}

EDescriptorHeapTypeDX12 g_getDX12DescriptorHeapType(D3D12_DESCRIPTOR_HEAP_TYPE heapType) {
    return getEnumMapping(heapTypeReverseMapping, heapType, EDescriptorHeapTypeDX12::CBV_SRV_UAV);
}

// ------------------- SHADER BINDING TYPE -------------------

static const std::unordered_map<EShaderBindingType, D3D12_DESCRIPTOR_RANGE_TYPE> bindingTypeMapping = {
    {EShaderBindingType::UNIFORMBUFFER,              D3D12_DESCRIPTOR_RANGE_TYPE_CBV},
    {EShaderBindingType::UNIFORMBUFFER_DYNAMIC,      D3D12_DESCRIPTOR_RANGE_TYPE_CBV},
    {EShaderBindingType::TEXTURE_SAMPLER_SRV,        D3D12_DESCRIPTOR_RANGE_TYPE_SRV},
    {EShaderBindingType::TEXTURE_SRV,                D3D12_DESCRIPTOR_RANGE_TYPE_SRV},
    {EShaderBindingType::TEXTURE_UAV,                D3D12_DESCRIPTOR_RANGE_TYPE_UAV},
    {EShaderBindingType::TEXTURE_ARRAY_SRV,          D3D12_DESCRIPTOR_RANGE_TYPE_SRV},
    {EShaderBindingType::SAMPLER,                    D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER},
    {EShaderBindingType::BUFFER_SRV,                 D3D12_DESCRIPTOR_RANGE_TYPE_SRV},
    {EShaderBindingType::BUFFER_UAV,                 D3D12_DESCRIPTOR_RANGE_TYPE_UAV},
    {EShaderBindingType::BUFFER_UAV_DYNAMIC,         D3D12_DESCRIPTOR_RANGE_TYPE_UAV},
    {EShaderBindingType::BUFFER_TEXEL_SRV,           D3D12_DESCRIPTOR_RANGE_TYPE_SRV},
    {EShaderBindingType::BUFFER_TEXEL_UAV,           D3D12_DESCRIPTOR_RANGE_TYPE_UAV},
    {EShaderBindingType::ACCELERATION_STRUCTURE_SRV, D3D12_DESCRIPTOR_RANGE_TYPE_SRV},
    {EShaderBindingType::SUBPASS_INPUT_ATTACHMENT,   (D3D12_DESCRIPTOR_RANGE_TYPE)-1}
};

static const std::unordered_map<D3D12_DESCRIPTOR_RANGE_TYPE, EShaderBindingType> bindingTypeReverseMapping = reverseMap(bindingTypeMapping);

D3D12_DESCRIPTOR_RANGE_TYPE g_getDX12ShaderBindingType(EShaderBindingType bindingType) {
    return getEnumMapping(bindingTypeMapping, bindingType, D3D12_DESCRIPTOR_RANGE_TYPE_CBV);
}

EShaderBindingType g_getDX12ShaderBindingType(D3D12_DESCRIPTOR_RANGE_TYPE bindingType) {
    return getEnumMapping(bindingTypeReverseMapping, bindingType, EShaderBindingType::UNIFORMBUFFER);
}

// ------------------- TEXTURE ADDRESS MODE -------------------

static const std::unordered_map<ETextureAddressMode, D3D12_TEXTURE_ADDRESS_MODE> addressModeMapping = {
    {ETextureAddressMode::REPEAT,               D3D12_TEXTURE_ADDRESS_MODE_WRAP},
    {ETextureAddressMode::MIRRORED_REPEAT,      D3D12_TEXTURE_ADDRESS_MODE_MIRROR},
    {ETextureAddressMode::CLAMP_TO_EDGE,        D3D12_TEXTURE_ADDRESS_MODE_CLAMP},
    {ETextureAddressMode::CLAMP_TO_BORDER,      D3D12_TEXTURE_ADDRESS_MODE_BORDER},
    {ETextureAddressMode::MIRROR_CLAMP_TO_EDGE, D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE}
};

static const std::unordered_map<D3D12_TEXTURE_ADDRESS_MODE, ETextureAddressMode> addressModeReverseMapping = reverseMap(addressModeMapping);

D3D12_TEXTURE_ADDRESS_MODE g_getDX12TextureAddressMode(ETextureAddressMode addressMode) {
    return getEnumMapping(addressModeMapping, addressMode, D3D12_TEXTURE_ADDRESS_MODE_WRAP);
}

ETextureAddressMode g_getDX12TextureAddressMode(D3D12_TEXTURE_ADDRESS_MODE addressMode) {
    return getEnumMapping(addressModeReverseMapping, addressMode, ETextureAddressMode::REPEAT);
}

// ------------------- COMPARE OP -------------------

static const std::unordered_map<ECompareOp, D3D12_COMPARISON_FUNC> compareOpMapping = {
    {ECompareOp::NEVER,     D3D12_COMPARISON_FUNC_NEVER},
    {ECompareOp::LESS,      D3D12_COMPARISON_FUNC_LESS},
    {ECompareOp::EQUAL,     D3D12_COMPARISON_FUNC_EQUAL},
    {ECompareOp::LEQUAL,    D3D12_COMPARISON_FUNC_LESS_EQUAL},
    {ECompareOp::GREATER,   D3D12_COMPARISON_FUNC_GREATER},
    {ECompareOp::NOTEQUAL,  D3D12_COMPARISON_FUNC_NOT_EQUAL},
    {ECompareOp::GEQUAL,    D3D12_COMPARISON_FUNC_GREATER_EQUAL},
    {ECompareOp::ALWAYS,    D3D12_COMPARISON_FUNC_ALWAYS}
};

static const std::unordered_map<D3D12_COMPARISON_FUNC, ECompareOp> compareOpReverseMapping = reverseMap(compareOpMapping);

D3D12_COMPARISON_FUNC g_getDX12CompareOp(ECompareOp compareOp) {
    return getEnumMapping(compareOpMapping, compareOp, D3D12_COMPARISON_FUNC_NEVER);
}

ECompareOp g_getDX12CompareOp(D3D12_COMPARISON_FUNC compareOp) {
    return getEnumMapping(compareOpReverseMapping, compareOp, ECompareOp::NEVER);
}

// ------------------- STENCIL OP -------------------

static const std::unordered_map<EStencilOp, D3D12_STENCIL_OP> stencilOpMapping = {
    {EStencilOp::KEEP,       D3D12_STENCIL_OP_KEEP},
    {EStencilOp::ZERO,       D3D12_STENCIL_OP_ZERO},
    {EStencilOp::REPLACE,    D3D12_STENCIL_OP_REPLACE},
    {EStencilOp::INCR,       D3D12_STENCIL_OP_INCR_SAT},
    {EStencilOp::INCR_WRAP,  D3D12_STENCIL_OP_INCR},
    {EStencilOp::DECR,       D3D12_STENCIL_OP_DECR_SAT},
    {EStencilOp::DECR_WRAP,  D3D12_STENCIL_OP_DECR},
    {EStencilOp::INVERT,     D3D12_STENCIL_OP_INVERT}
};

static const std::unordered_map<D3D12_STENCIL_OP, EStencilOp> stencilOpReverseMapping = reverseMap(stencilOpMapping);

D3D12_STENCIL_OP g_getDX12StencilOp(EStencilOp stencilOp) {
    return getEnumMapping(stencilOpMapping, stencilOp, D3D12_STENCIL_OP_KEEP);
}

EStencilOp g_getDX12StencilOp(D3D12_STENCIL_OP stencilOp) {
    return getEnumMapping(stencilOpReverseMapping, stencilOp, EStencilOp::KEEP);
}

// ------------------- PRIMITIVE TOPOLOGY TYPE -------------------

static const std::unordered_map<EPrimitiveType, D3D12_PRIMITIVE_TOPOLOGY_TYPE> primitiveTopologyTypeMapping = {
    {EPrimitiveType::POINTS,                   D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT},
    {EPrimitiveType::LINES,                    D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE},
    {EPrimitiveType::LINES_ADJACENCY,          D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE},
    {EPrimitiveType::LINE_STRIP_ADJACENCY,     D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE},
    {EPrimitiveType::TRIANGLES,                D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE},
    {EPrimitiveType::TRIANGLE_STRIP,           D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE},
    {EPrimitiveType::TRIANGLES_ADJACENCY,      D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE},
    {EPrimitiveType::TRIANGLE_STRIP_ADJACENCY, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE}
};

static const std::unordered_map<D3D12_PRIMITIVE_TOPOLOGY_TYPE, EPrimitiveType> primitiveTopologyTypeReverseMapping = reverseMap(primitiveTopologyTypeMapping);

D3D12_PRIMITIVE_TOPOLOGY_TYPE g_getDX12PrimitiveTopologyTypeOnly(EPrimitiveType type) {
    return getEnumMapping(primitiveTopologyTypeMapping, type, D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED);
}

EPrimitiveType g_getDX12PrimitiveTopologyTypeOnly(D3D12_PRIMITIVE_TOPOLOGY_TYPE type) {
    return getEnumMapping(primitiveTopologyTypeReverseMapping, type, EPrimitiveType::POINTS);
}

// ------------------- PRIMITIVE TOPOLOGY -------------------

static const std::unordered_map<EPrimitiveType, D3D_PRIMITIVE_TOPOLOGY> primitiveTopologyMapping = {
    {EPrimitiveType::POINTS,                   D3D_PRIMITIVE_TOPOLOGY_POINTLIST},
    {EPrimitiveType::LINES,                    D3D_PRIMITIVE_TOPOLOGY_LINELIST},
    {EPrimitiveType::LINES_ADJACENCY,          D3D_PRIMITIVE_TOPOLOGY_LINELIST_ADJ},
    {EPrimitiveType::LINE_STRIP_ADJACENCY,     D3D_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ},
    {EPrimitiveType::TRIANGLES,                D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST},
    {EPrimitiveType::TRIANGLE_STRIP,           D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP},
    {EPrimitiveType::TRIANGLES_ADJACENCY,      D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ},
    {EPrimitiveType::TRIANGLE_STRIP_ADJACENCY, D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ}
};

static const std::unordered_map<D3D_PRIMITIVE_TOPOLOGY, EPrimitiveType> primitiveTopologyReverseMapping = reverseMap(primitiveTopologyMapping);

D3D_PRIMITIVE_TOPOLOGY g_getDX12PrimitiveTopology(EPrimitiveType type) {
    return getEnumMapping(primitiveTopologyMapping, type, D3D_PRIMITIVE_TOPOLOGY_UNDEFINED);
}

EPrimitiveType g_getDX12PrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY type) {
    return getEnumMapping(primitiveTopologyReverseMapping, type, EPrimitiveType::POINTS);
}

// ------------------- VERTEX INPUT RATE -------------------

static const std::unordered_map<EVertexInputRate, D3D12_INPUT_CLASSIFICATION> vertexInputRateMapping = {
    {EVertexInputRate::VERTEX,   D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA},
    {EVertexInputRate::INSTANCE, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA}
};

static const std::unordered_map<D3D12_INPUT_CLASSIFICATION, EVertexInputRate> vertexInputRateReverseMapping = reverseMap(vertexInputRateMapping);

D3D12_INPUT_CLASSIFICATION g_getDX12VertexInputRate(EVertexInputRate inputRate) {
    return getEnumMapping(vertexInputRateMapping, inputRate, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA);
}

EVertexInputRate g_getDX12VertexInputRate(D3D12_INPUT_CLASSIFICATION inputRate) {
    return getEnumMapping(vertexInputRateReverseMapping, inputRate, EVertexInputRate::VERTEX);
}

// ------------------- POLYGON MODE -------------------

static const std::unordered_map<EPolygonMode, D3D12_FILL_MODE> polygonModeMapping = {
    {EPolygonMode::POINT, D3D12_FILL_MODE_WIREFRAME},  // Not supported, fallback to wireframe
    {EPolygonMode::LINE,  D3D12_FILL_MODE_WIREFRAME},
    {EPolygonMode::FILL,  D3D12_FILL_MODE_SOLID}
};

static const std::unordered_map<D3D12_FILL_MODE, EPolygonMode> polygonModeReverseMapping = reverseMap(polygonModeMapping);

D3D12_FILL_MODE g_getDX12PolygonMode(EPolygonMode polygonMode) {
    return getEnumMapping(polygonModeMapping, polygonMode, D3D12_FILL_MODE_SOLID);
}

EPolygonMode g_getDX12PolygonMode(D3D12_FILL_MODE polygonMode) {
    return getEnumMapping(polygonModeReverseMapping, polygonMode, EPolygonMode::FILL);
}

// ------------------- CULL MODE -------------------

static const std::unordered_map<ECullMode, D3D12_CULL_MODE> cullModeMapping = {
    {ECullMode::NONE,           D3D12_CULL_MODE_NONE},
    {ECullMode::BACK,           D3D12_CULL_MODE_BACK},
    {ECullMode::FRONT,          D3D12_CULL_MODE_FRONT},
    {ECullMode::FRONT_AND_BACK, D3D12_CULL_MODE_BACK}  // Not supported, fallback to BACK
};

static const std::unordered_map<D3D12_CULL_MODE, ECullMode> cullModeReverseMapping = reverseMap(cullModeMapping);

D3D12_CULL_MODE g_getDX12CullMode(ECullMode cullMode) {
    return getEnumMapping(cullModeMapping, cullMode, D3D12_CULL_MODE_NONE);
}

ECullMode g_getDX12CullMode(D3D12_CULL_MODE cullMode) {
    return getEnumMapping(cullModeReverseMapping, cullMode, ECullMode::NONE);
}

// ------------------- BLEND FACTOR -------------------

static const std::unordered_map<EBlendFactor, D3D12_BLEND> blendFactorMapping = {
    {EBlendFactor::ZERO,                    D3D12_BLEND_ZERO},
    {EBlendFactor::ONE,                     D3D12_BLEND_ONE},
    {EBlendFactor::SRC_COLOR,               D3D12_BLEND_SRC_COLOR},
    {EBlendFactor::ONE_MINUS_SRC_COLOR,     D3D12_BLEND_INV_SRC_COLOR},
    {EBlendFactor::DST_COLOR,               D3D12_BLEND_DEST_COLOR},
    {EBlendFactor::ONE_MINUS_DST_COLOR,     D3D12_BLEND_INV_DEST_COLOR},
    {EBlendFactor::SRC_ALPHA,               D3D12_BLEND_SRC_ALPHA},
    {EBlendFactor::ONE_MINUS_SRC_ALPHA,     D3D12_BLEND_INV_SRC_ALPHA},
    {EBlendFactor::DST_ALPHA,               D3D12_BLEND_DEST_ALPHA},
    {EBlendFactor::ONE_MINUS_DST_ALPHA,     D3D12_BLEND_INV_DEST_ALPHA},
    {EBlendFactor::CONSTANT_COLOR,          D3D12_BLEND_BLEND_FACTOR},
    {EBlendFactor::ONE_MINUS_CONSTANT_COLOR,D3D12_BLEND_INV_BLEND_FACTOR},
    {EBlendFactor::SRC_ALPHA_SATURATE,      D3D12_BLEND_SRC_ALPHA_SAT}
};

static const std::unordered_map<D3D12_BLEND, EBlendFactor> blendFactorReverseMapping = reverseMap(blendFactorMapping);

D3D12_BLEND g_getDX12BlendFactor(EBlendFactor type) {
    return getEnumMapping(blendFactorMapping, type, D3D12_BLEND_ZERO);
}

EBlendFactor g_getDX12BlendFactor(D3D12_BLEND type) {
    return getEnumMapping(blendFactorReverseMapping, type, EBlendFactor::ZERO);
}

// ------------------- BLEND OP -------------------

static const std::unordered_map<EBlendOp, D3D12_BLEND_OP> blendOpMapping = {
    {EBlendOp::ADD,               D3D12_BLEND_OP_ADD},
    {EBlendOp::SUBTRACT,          D3D12_BLEND_OP_SUBTRACT},
    {EBlendOp::REVERSE_SUBTRACT,  D3D12_BLEND_OP_REV_SUBTRACT},
    {EBlendOp::MIN_VALUE,         D3D12_BLEND_OP_MIN},
    {EBlendOp::MAX_VALUE,         D3D12_BLEND_OP_MAX}
};

static const std::unordered_map<D3D12_BLEND_OP, EBlendOp> blendOpReverseMapping = reverseMap(blendOpMapping);

D3D12_BLEND_OP g_getDX12BlendOp(EBlendOp blendOp) {
    return getEnumMapping(blendOpMapping, blendOp, D3D12_BLEND_OP_ADD);
}

EBlendOp g_getDX12BlendOp(D3D12_BLEND_OP blendOp) {
    return getEnumMapping(blendOpReverseMapping, blendOp, EBlendOp::ADD);
}

// ------------------- RESOURCE LAYOUT -------------------

static const std::unordered_map<EResourceLayout, D3D12_RESOURCE_STATES> resourceLayoutMapping = {
    {EResourceLayout::UNDEFINED,                           D3D12_RESOURCE_STATE_COMMON},
    {EResourceLayout::GENERAL,                             D3D12_RESOURCE_STATE_COMMON},
    {EResourceLayout::UAV,                                 D3D12_RESOURCE_STATE_UNORDERED_ACCESS},
    {EResourceLayout::COLOR_ATTACHMENT,                    D3D12_RESOURCE_STATE_RENDER_TARGET},
    {EResourceLayout::DEPTH_STENCIL_ATTACHMENT,            D3D12_RESOURCE_STATE_DEPTH_WRITE},
    {EResourceLayout::DEPTH_STENCIL_READ_ONLY,             D3D12_RESOURCE_STATE_DEPTH_READ},
    {EResourceLayout::SHADER_READ_ONLY,                    D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE 
                                                          | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE},
    {EResourceLayout::TRANSFER_SRC,                        D3D12_RESOURCE_STATE_COPY_SOURCE},
    {EResourceLayout::TRANSFER_DST,                        D3D12_RESOURCE_STATE_COPY_DEST},
    {EResourceLayout::PREINITIALIZED,                      D3D12_RESOURCE_STATE_COMMON},
    {EResourceLayout::DEPTH_READ_ONLY_STENCIL_ATTACHMENT,  D3D12_RESOURCE_STATE_DEPTH_WRITE},
    {EResourceLayout::DEPTH_ATTACHMENT_STENCIL_READ_ONLY,  D3D12_RESOURCE_STATE_DEPTH_WRITE},
    {EResourceLayout::DEPTH_ATTACHMENT,                    D3D12_RESOURCE_STATE_DEPTH_WRITE},
    {EResourceLayout::DEPTH_READ_ONLY,                     D3D12_RESOURCE_STATE_DEPTH_READ 
                                                          | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE 
                                                          | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE},
    {EResourceLayout::STENCIL_ATTACHMENT,                  D3D12_RESOURCE_STATE_DEPTH_WRITE},
    {EResourceLayout::STENCIL_READ_ONLY,                   D3D12_RESOURCE_STATE_DEPTH_READ 
                                                          | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE 
                                                          | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE},
    {EResourceLayout::PRESENT_SRC,                         D3D12_RESOURCE_STATE_PRESENT},
    {EResourceLayout::SHARED_PRESENT,                      D3D12_RESOURCE_STATE_PRESENT},
    {EResourceLayout::SHADING_RATE_NV,                     D3D12_RESOURCE_STATE_SHADING_RATE_SOURCE},
    {EResourceLayout::FRAGMENT_DENSITY_MAP_EXT,            D3D12_RESOURCE_STATE_COMMON},
    {EResourceLayout::READ_ONLY,                           D3D12_RESOURCE_STATE_GENERIC_READ},
    {EResourceLayout::ATTACHMENT,                          D3D12_RESOURCE_STATE_RENDER_TARGET},
    {EResourceLayout::ACCELERATION_STRUCTURE,              D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE}
};

static const std::unordered_map<D3D12_RESOURCE_STATES, EResourceLayout> resourceLayoutReverseMapping = reverseMap(resourceLayoutMapping);

D3D12_RESOURCE_STATES g_getDX12ResourceLayout(EResourceLayout resourceLayout) {
    return getEnumMapping(resourceLayoutMapping, resourceLayout, D3D12_RESOURCE_STATE_COMMON);
}

EResourceLayout g_getDX12ResourceLayout(D3D12_RESOURCE_STATES resourceState) {
    return getEnumMapping(resourceLayoutReverseMapping, resourceState, EResourceLayout::UNDEFINED);
}

// clang-format on



void CreatedResource::free() {
  if (m_resource_) {
    if (m_resourceType_ == CreatedResource::EType::Standalone) {
      if (g_rhiDx12) {
        g_rhiDx12->m_deallocatorMultiFrameStandaloneResource_.free(m_resource_);
      }
    } else if (m_resourceType_ == CreatedResource::EType::ResourcePool) {
      if (g_rhiDx12) {
        g_rhiDx12->m_deallocatorMultiFramePlacedResource_.free(m_resource_);
      }
    } else if (m_resourceType_ == CreatedResource::EType::Swapchain) {
      // Nothing to do
    } else {
      assert(0);
    }
  }
}

}  // namespace game_engine

#endif  // GAME_ENGINE_RHI_DX12