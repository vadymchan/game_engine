#ifndef GAME_ENGINE_RHI_TYPE_VK_H
#define GAME_ENGINE_RHI_TYPE_VK_H

#include "gfx/rhi/rhi_type.h"

#include <vulkan/vulkan.h>

namespace game_engine {

// clang-format off

VkFilter g_getVulkanTextureFilterType(ETextureFilter textureFilter);
ETextureFilter g_getVulkanTextureFilterType(VkFilter filter);

VkSamplerAddressMode g_getVulkanTextureAddressMode(ETextureAddressMode addressMode);
ETextureAddressMode g_getVulkanTextureAddressMode(VkSamplerAddressMode addressMode);

VkSamplerMipmapMode g_getVulkanTextureMipmapMode(ETextureFilter textureFilter);
ETextureFilter g_getVulkanTextureMipmapMode(VkSamplerMipmapMode mipmapMode);

VkFormat g_getVulkanTextureFormat(ETextureFormat textureFormat);
ETextureFormat g_getVulkanTextureFormat(VkFormat format);

//inline VkFilter g_getVulkanTextureFilterType(ETextureFilter textureFilter) {
//    static const std::unordered_map<ETextureFilter, VkFilter> filterMapping = {
//        {ETextureFilter::NEAREST,                VK_FILTER_NEAREST},
//        {ETextureFilter::LINEAR,                 VK_FILTER_LINEAR},
//        {ETextureFilter::NEAREST_MIPMAP_NEAREST, VK_FILTER_NEAREST},
//        {ETextureFilter::LINEAR_MIPMAP_NEAREST,  VK_FILTER_LINEAR},
//        {ETextureFilter::NEAREST_MIPMAP_LINEAR,  VK_FILTER_NEAREST},
//        {ETextureFilter::LINEAR_MIPMAP_LINEAR,   VK_FILTER_LINEAR}
//    };
//
//    return getEnumMapping(textureFilter, filterMapping, VK_FILTER_MAX_ENUM);
//}
//
//inline VkSamplerAddressMode g_getVulkanTextureAddressMode(ETextureAddressMode addressMode) {
//    static const std::unordered_map<ETextureAddressMode, VkSamplerAddressMode> addressModeMapping = {
//        {ETextureAddressMode::REPEAT,               VK_SAMPLER_ADDRESS_MODE_REPEAT},
//        {ETextureAddressMode::MIRRORED_REPEAT,      VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT},
//        {ETextureAddressMode::CLAMP_TO_EDGE,        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE},
//        {ETextureAddressMode::CLAMP_TO_BORDER,      VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER},
//        {ETextureAddressMode::MIRROR_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE}
//    };
//
//    return getEnumMapping(addressMode, addressModeMapping, VK_SAMPLER_ADDRESS_MODE_MAX_ENUM);
//}
//
//// TODO: not used + seems to be wrong
//inline VkSamplerMipmapMode g_getVulkanTextureMipmapMode(ETextureFilter textureFilter) {
//    static const std::unordered_map<ETextureFilter, VkSamplerMipmapMode> mipmapModeMapping = {
//        {ETextureFilter::NEAREST,                VK_SAMPLER_MIPMAP_MODE_NEAREST},
//        {ETextureFilter::LINEAR,                 VK_SAMPLER_MIPMAP_MODE_NEAREST},
//        {ETextureFilter::NEAREST_MIPMAP_NEAREST, VK_SAMPLER_MIPMAP_MODE_NEAREST},
//        {ETextureFilter::LINEAR_MIPMAP_NEAREST,  VK_SAMPLER_MIPMAP_MODE_NEAREST},
//        {ETextureFilter::NEAREST_MIPMAP_LINEAR,  VK_SAMPLER_MIPMAP_MODE_LINEAR},
//        {ETextureFilter::LINEAR_MIPMAP_LINEAR,   VK_SAMPLER_MIPMAP_MODE_LINEAR}
//    };
//
//    return getEnumMapping(textureFilter, mipmapModeMapping, VK_SAMPLER_MIPMAP_MODE_MAX_ENUM);
//}
//
//inline VkFormat g_getVulkanTextureFormat(ETextureFormat textureFormat) {
//    static const std::unordered_map<ETextureFormat, VkFormat> formatMapping = {
//        {ETextureFormat::RGB8,       VK_FORMAT_R8G8B8_UNORM},
//        {ETextureFormat::RGB32F,     VK_FORMAT_R32G32B32_SFLOAT},
//        {ETextureFormat::RGB16F,     VK_FORMAT_R16G16B16_SFLOAT},
//        {ETextureFormat::R11G11B10F, VK_FORMAT_B10G11R11_UFLOAT_PACK32},
//        {ETextureFormat::RGBA8,      VK_FORMAT_R8G8B8A8_UNORM},
//        {ETextureFormat::RGBA16F,    VK_FORMAT_R16G16B16A16_SFLOAT},
//        {ETextureFormat::RGBA32F,    VK_FORMAT_R32G32B32A32_SFLOAT},
//        {ETextureFormat::RGBA8SI,    VK_FORMAT_R8G8B8A8_SINT},
//        {ETextureFormat::RGBA8UI,    VK_FORMAT_R8G8B8A8_UINT},
//        {ETextureFormat::BGRA8,      VK_FORMAT_B8G8R8A8_UNORM},
//        {ETextureFormat::R8,         VK_FORMAT_R8_UNORM},
//        {ETextureFormat::R16F,       VK_FORMAT_R16_SFLOAT},
//        {ETextureFormat::R32F,       VK_FORMAT_R32_SFLOAT},
//        {ETextureFormat::R8UI,       VK_FORMAT_R8_UINT},
//        {ETextureFormat::R32UI,      VK_FORMAT_R32_UINT},
//        {ETextureFormat::RG8,        VK_FORMAT_R8G8_UNORM},
//        {ETextureFormat::RG16F,      VK_FORMAT_R16G16_SFLOAT},
//        {ETextureFormat::RG32F,      VK_FORMAT_R32G32_SFLOAT},
//        {ETextureFormat::D16,        VK_FORMAT_D16_UNORM},
//        {ETextureFormat::D16_S8,     VK_FORMAT_D16_UNORM_S8_UINT},
//        {ETextureFormat::D24,        VK_FORMAT_X8_D24_UNORM_PACK32},
//        {ETextureFormat::D24_S8,     VK_FORMAT_D24_UNORM_S8_UINT},
//        {ETextureFormat::D32,        VK_FORMAT_D32_SFLOAT},
//        {ETextureFormat::D32_S8,     VK_FORMAT_D32_SFLOAT_S8_UINT},
//        {ETextureFormat::BC1_UNORM,  VK_FORMAT_BC1_RGBA_UNORM_BLOCK},
//        {ETextureFormat::BC2_UNORM,  VK_FORMAT_BC2_UNORM_BLOCK},
//        {ETextureFormat::BC3_UNORM,  VK_FORMAT_BC3_UNORM_BLOCK},
//        {ETextureFormat::BC4_UNORM,  VK_FORMAT_BC4_UNORM_BLOCK},
//        {ETextureFormat::BC4_SNORM,  VK_FORMAT_BC4_SNORM_BLOCK},
//        {ETextureFormat::BC5_UNORM,  VK_FORMAT_BC5_UNORM_BLOCK},
//        {ETextureFormat::BC5_SNORM,  VK_FORMAT_BC5_SNORM_BLOCK},
//        {ETextureFormat::BC6H_UF16,  VK_FORMAT_BC6H_UFLOAT_BLOCK},
//        {ETextureFormat::BC6H_SF16,  VK_FORMAT_BC6H_SFLOAT_BLOCK},
//        {ETextureFormat::BC7_UNORM,  VK_FORMAT_BC7_UNORM_BLOCK}
//    };
//
//    return getEnumMapping(textureFormat, formatMapping, VK_FORMAT_UNDEFINED);
//}

inline auto g_getVulkanTextureComponentCount(ETextureFormat type) {
    static const std::unordered_map<ETextureFormat, int> textureComponentCountMapping = {
        {ETextureFormat::RGB8,           3},
        {ETextureFormat::RGB32F,         3},
        {ETextureFormat::RGB16F,         3},
        {ETextureFormat::R11G11B10F,     3},

        {ETextureFormat::RGBA8,          4},
        {ETextureFormat::RGBA16F,        4},
        {ETextureFormat::RGBA32F,        4},
        {ETextureFormat::RGBA8SI,        4},
        {ETextureFormat::RGBA8UI,        4},

        {ETextureFormat::BGRA8,          4},

        {ETextureFormat::R8,             1},
        {ETextureFormat::R16F,           1},
        {ETextureFormat::R32F,           1},
        {ETextureFormat::R8UI,           1},
        {ETextureFormat::R32UI,          1},

        {ETextureFormat::RG8,            2},
        {ETextureFormat::RG16F,          2},
        {ETextureFormat::RG32F,          2},

        {ETextureFormat::D16,            1},
        {ETextureFormat::D16_S8,         2},
        {ETextureFormat::D24,            1},
        {ETextureFormat::D24_S8,         2},
        {ETextureFormat::D32,            1},
        {ETextureFormat::D32_S8,         2}
    };

    return getEnumMapping(textureComponentCountMapping, type, 0);  
}

inline auto g_getVulkanTexturePixelSize(ETextureFormat type) {
    static const std::unordered_map<ETextureFormat, int> texturePixelSizeMapping = {
        {ETextureFormat::RGB8,           3},
        {ETextureFormat::RGB32F,         12},
        {ETextureFormat::RGB16F,         6},
        {ETextureFormat::R11G11B10F,     4},

        {ETextureFormat::RGBA8,          4},
        {ETextureFormat::RGBA16F,        8},
        {ETextureFormat::RGBA32F,        16},
        {ETextureFormat::RGBA8SI,        4},
        {ETextureFormat::RGBA8UI,        4},

        {ETextureFormat::BGRA8,          4},

        {ETextureFormat::R8,             1},
        {ETextureFormat::R16F,           2},
        {ETextureFormat::R32F,           4},
        {ETextureFormat::R8UI,           1},
        {ETextureFormat::R32UI,          4},

        {ETextureFormat::RG8,            2},
        {ETextureFormat::RG16F,          2},
        {ETextureFormat::RG32F,          4},

        {ETextureFormat::D16,            2},
        {ETextureFormat::D16_S8,         3},
        {ETextureFormat::D24,            3},
        {ETextureFormat::D24_S8,         4},
        {ETextureFormat::D32,            4},
        {ETextureFormat::D32_S8,         5}
    };

    return getEnumMapping(texturePixelSizeMapping, type, 0); 
}

inline void g_getVulkanAttachmentLoadStoreOp(VkAttachmentLoadOp&    loadOp,
                                           VkAttachmentStoreOp&   storeOp,
                                           EAttachmentLoadStoreOp type) {
  switch (type) {
    case EAttachmentLoadStoreOp::LOAD_STORE:
      loadOp  = VK_ATTACHMENT_LOAD_OP_LOAD;
      storeOp = VK_ATTACHMENT_STORE_OP_STORE;
      break;
    case EAttachmentLoadStoreOp::LOAD_DONTCARE:
      loadOp  = VK_ATTACHMENT_LOAD_OP_LOAD;
      storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
      break;
    case EAttachmentLoadStoreOp::CLEAR_STORE:
      loadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
      storeOp = VK_ATTACHMENT_STORE_OP_STORE;
      break;
    case EAttachmentLoadStoreOp::CLEAR_DONTCARE:
      loadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
      storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
      break;
    case EAttachmentLoadStoreOp::DONTCARE_STORE:
      loadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
      storeOp = VK_ATTACHMENT_STORE_OP_STORE;
      break;
    case EAttachmentLoadStoreOp::DONTCARE_DONTCARE:
      loadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
      storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
      break;
    case EAttachmentLoadStoreOp::MAX:
    default:
      assert(0);
      break;
  }
}

inline auto g_getVulkanShaderAccessFlags(EShaderAccessStageFlag type) {
  switch (type) {
    case EShaderAccessStageFlag::VERTEX:
      return VK_SHADER_STAGE_VERTEX_BIT;
    case EShaderAccessStageFlag::TESSELLATION_CONTROL:
      return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    case EShaderAccessStageFlag::TESSELLATION_EVALUATION:
      return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    case EShaderAccessStageFlag::GEOMETRY:
      return VK_SHADER_STAGE_GEOMETRY_BIT;
    case EShaderAccessStageFlag::FRAGMENT:
      return VK_SHADER_STAGE_FRAGMENT_BIT;
    case EShaderAccessStageFlag::COMPUTE:
      return VK_SHADER_STAGE_COMPUTE_BIT;
    case EShaderAccessStageFlag::ALL_GRAPHICS:
      return VK_SHADER_STAGE_ALL_GRAPHICS;
    case EShaderAccessStageFlag::RAYTRACING_RAYGEN:
      return VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    case EShaderAccessStageFlag::RAYTRACING_ANYHIT:
      return VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
    case EShaderAccessStageFlag::RAYTRACING_CLOSESTHIT:
      return VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
    case EShaderAccessStageFlag::RAYTRACING_MISS:
      return VK_SHADER_STAGE_MISS_BIT_KHR;
    case EShaderAccessStageFlag::ALL:
    case EShaderAccessStageFlag::ALL_RAYTRACING:
      return VK_SHADER_STAGE_ALL;
    default:
      assert(0);
      break;
  }
  return VK_SHADER_STAGE_ALL;
}

// clang-format off

VkPrimitiveTopology g_getVulkanPrimitiveTopology(EPrimitiveType primitiveType);
EPrimitiveType g_getVulkanPrimitiveTopology(VkPrimitiveTopology primitiveTopology);

VkVertexInputRate g_getVulkanVertexInputRate(EVertexInputRate inputRate);
EVertexInputRate g_getVulkanVertexInputRate(VkVertexInputRate inputRate);

VkPolygonMode g_getVulkanPolygonMode(EPolygonMode polygonMode);
EPolygonMode g_getVulkanPolygonMode(VkPolygonMode polygonMode);

//inline VkPrimitiveTopology g_getVulkanPrimitiveTopology(EPrimitiveType primitiveType) {
//    static const std::unordered_map<EPrimitiveType, VkPrimitiveTopology> topologyMapping = {
//        {EPrimitiveType::POINTS,                   VK_PRIMITIVE_TOPOLOGY_POINT_LIST},
//        {EPrimitiveType::LINES,                    VK_PRIMITIVE_TOPOLOGY_LINE_LIST},
//        {EPrimitiveType::LINES_ADJACENCY,          VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY},
//        {EPrimitiveType::LINE_STRIP_ADJACENCY,     VK_PRIMITIVE_TOPOLOGY_LINE_STRIP},
//        {EPrimitiveType::TRIANGLES,                VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST},
//        {EPrimitiveType::TRIANGLES_ADJACENCY,      VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY},
//        {EPrimitiveType::TRIANGLE_STRIP_ADJACENCY, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY},
//        {EPrimitiveType::TRIANGLE_STRIP,           VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP}
//    };
//
//    return getEnumMapping(primitiveType, topologyMapping, VK_PRIMITIVE_TOPOLOGY_MAX_ENUM);
//}
//
//inline VkVertexInputRate g_getVulkanVertexInputRate(EVertexInputRate inputRate) {
//    static const std::unordered_map<EVertexInputRate, VkVertexInputRate> inputRateMapping = {
//        {EVertexInputRate::VERTEX,   VK_VERTEX_INPUT_RATE_VERTEX},
//        {EVertexInputRate::INSTANCE, VK_VERTEX_INPUT_RATE_INSTANCE}
//    };
//
//    return getEnumMapping(inputRate, inputRateMapping, VK_VERTEX_INPUT_RATE_MAX_ENUM);
//}
//
//inline VkPolygonMode g_getVulkanPolygonMode(EPolygonMode polygonMode) {
//    static const std::unordered_map<EPolygonMode, VkPolygonMode> polygonModeMapping = {
//        {EPolygonMode::POINT, VK_POLYGON_MODE_POINT},
//        {EPolygonMode::LINE,  VK_POLYGON_MODE_LINE},
//        {EPolygonMode::FILL,  VK_POLYGON_MODE_FILL}
//    };
//
//    return getEnumMapping(polygonMode, polygonModeMapping, VK_POLYGON_MODE_MAX_ENUM);  
//}


//VkCullModeFlags g_getVulkanCullMode(ECullMode cullMode) {
//    static const std::unordered_map<ECullMode, VkCullModeFlags> cullModeMapping = {
//        {ECullMode::NONE,           VK_CULL_MODE_NONE},
//        {ECullMode::BACK,           VK_CULL_MODE_BACK_BIT},
//        {ECullMode::FRONT,          VK_CULL_MODE_FRONT_BIT},
//        {ECullMode::FRONT_AND_BACK, VK_CULL_MODE_FRONT_AND_BACK}
//    };
//
//    return getEnumMapping(cullMode, cullModeMapping, VK_CULL_MODE_FLAG_BITS_MAX_ENUM);  
//}

inline VkCullModeFlags g_getVulkanCullMode(ECullMode cullMode) {
    switch (cullMode) {
        case ECullMode::NONE:
            return VK_CULL_MODE_NONE;
        case ECullMode::BACK:
            return VK_CULL_MODE_BACK_BIT;
        case ECullMode::FRONT:
            return VK_CULL_MODE_FRONT_BIT;
        case ECullMode::FRONT_AND_BACK:
            return VK_CULL_MODE_FRONT_AND_BACK;
        default:
            assert(0); 
            break;
    }
    return VK_CULL_MODE_FLAG_BITS_MAX_ENUM;
}

VkFrontFace g_getVulkanFrontFace(EFrontFace frontFace);
EFrontFace g_getVulkanFrontFace(VkFrontFace frontFace);

VkStencilOp g_getVulkanStencilOp(EStencilOp stencilOp);
EStencilOp g_getVulkanStencilOp(VkStencilOp stencilOp);

VkCompareOp g_getVulkanCompareOp(ECompareOp compareOp);
ECompareOp g_getVulkanCompareOp(VkCompareOp compareOp);

VkBlendFactor g_getVulkanBlendFactor(EBlendFactor blendFactor);
EBlendFactor g_getVulkanBlendFactor(VkBlendFactor blendFactor);

VkBlendOp g_getVulkanBlendOp(EBlendOp blendOp);
EBlendOp g_getVulkanBlendOp(VkBlendOp blendOp);

VkImageLayout g_getVulkanImageLayout(EResourceLayout resourceLayout);
EResourceLayout g_getVulkanImageLayout(VkImageLayout resourceLayout);

VkDynamicState g_getVulkanPipelineDynamicState(EPipelineDynamicState dynamicState);
EPipelineDynamicState g_getVulkanPipelineDynamicState(VkDynamicState dynamicState);


//inline VkFrontFace g_getVulkanFrontFace(EFrontFace frontFace) {
//    static const std::unordered_map<EFrontFace, VkFrontFace> frontFaceMapping = {
//        {EFrontFace::CW,  VK_FRONT_FACE_CLOCKWISE},
//        {EFrontFace::CCW, VK_FRONT_FACE_COUNTER_CLOCKWISE}
//    };
//
//    return getEnumMapping(frontFace, frontFaceMapping, VK_FRONT_FACE_MAX_ENUM);
//}
//
//inline VkStencilOp g_getVulkanStencilOp(EStencilOp stencilOp) {
//    static const std::unordered_map<EStencilOp, VkStencilOp> stencilOpMapping = {
//        {EStencilOp::KEEP,      VK_STENCIL_OP_KEEP},
//        {EStencilOp::ZERO,      VK_STENCIL_OP_ZERO},
//        {EStencilOp::REPLACE,   VK_STENCIL_OP_REPLACE},
//        {EStencilOp::INCR,      VK_STENCIL_OP_INCREMENT_AND_CLAMP},
//        {EStencilOp::INCR_WRAP, VK_STENCIL_OP_INCREMENT_AND_WRAP},
//        {EStencilOp::DECR,      VK_STENCIL_OP_DECREMENT_AND_CLAMP},
//        {EStencilOp::DECR_WRAP, VK_STENCIL_OP_DECREMENT_AND_WRAP},
//        {EStencilOp::INVERT,    VK_STENCIL_OP_INVERT}
//    };
//
//    return getEnumMapping(stencilOp, stencilOpMapping, VK_STENCIL_OP_MAX_ENUM);
//}
//
//
//inline VkCompareOp g_getVulkanCompareOp(ECompareOp compareOp) {
//    static const std::unordered_map<ECompareOp, VkCompareOp> compareOpMapping = {
//        {ECompareOp::NEVER,    VK_COMPARE_OP_NEVER},
//        {ECompareOp::LESS,     VK_COMPARE_OP_LESS},
//        {ECompareOp::EQUAL,    VK_COMPARE_OP_EQUAL},
//        {ECompareOp::LEQUAL,   VK_COMPARE_OP_LESS_OR_EQUAL},
//        {ECompareOp::GREATER,  VK_COMPARE_OP_GREATER},
//        {ECompareOp::NOTEQUAL, VK_COMPARE_OP_NOT_EQUAL},
//        {ECompareOp::GEQUAL,   VK_COMPARE_OP_GREATER_OR_EQUAL},
//        {ECompareOp::ALWAYS,   VK_COMPARE_OP_ALWAYS}
//    };
//
//    return getEnumMapping(compareOp, compareOpMapping, VK_COMPARE_OP_MAX_ENUM);
//}
//
//inline VkBlendFactor g_getVulkanBlendFactor(EBlendFactor blendFactor) {
//    static const std::unordered_map<EBlendFactor, VkBlendFactor> blendFactorMapping = {
//        {EBlendFactor::ZERO,                     VK_BLEND_FACTOR_ZERO},
//        {EBlendFactor::ONE,                      VK_BLEND_FACTOR_ONE},
//        {EBlendFactor::SRC_COLOR,                VK_BLEND_FACTOR_SRC_COLOR},
//        {EBlendFactor::ONE_MINUS_SRC_COLOR,      VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR},
//        {EBlendFactor::DST_COLOR,                VK_BLEND_FACTOR_DST_COLOR},
//        {EBlendFactor::ONE_MINUS_DST_COLOR,      VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR},
//        {EBlendFactor::SRC_ALPHA,                VK_BLEND_FACTOR_SRC_ALPHA},
//        {EBlendFactor::ONE_MINUS_SRC_ALPHA,      VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA},
//        {EBlendFactor::DST_ALPHA,                VK_BLEND_FACTOR_DST_ALPHA},
//        {EBlendFactor::ONE_MINUS_DST_ALPHA,      VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA},
//        {EBlendFactor::CONSTANT_COLOR,           VK_BLEND_FACTOR_CONSTANT_COLOR},
//        {EBlendFactor::ONE_MINUS_CONSTANT_COLOR, VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR},
//        {EBlendFactor::CONSTANT_ALPHA,           VK_BLEND_FACTOR_CONSTANT_ALPHA},
//        {EBlendFactor::ONE_MINUS_CONSTANT_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA},
//        {EBlendFactor::SRC_ALPHA_SATURATE,       VK_BLEND_FACTOR_SRC_ALPHA_SATURATE}
//    };
//
//    return getEnumMapping(blendFactor, blendFactorMapping, VK_BLEND_FACTOR_MAX_ENUM);
//}
//
//inline VkBlendOp g_getVulkanBlendOp(EBlendOp blendOp) {
//    static const std::unordered_map<EBlendOp, VkBlendOp> blendOpMapping = {
//        {EBlendOp::ADD,              VK_BLEND_OP_ADD},
//        {EBlendOp::SUBTRACT,         VK_BLEND_OP_SUBTRACT},
//        {EBlendOp::REVERSE_SUBTRACT, VK_BLEND_OP_REVERSE_SUBTRACT},
//        {EBlendOp::MIN_VALUE,        VK_BLEND_OP_MIN},
//        {EBlendOp::MAX_VALUE,        VK_BLEND_OP_MAX}
//    };
//
//    return getEnumMapping(blendOp, blendOpMapping, VK_BLEND_OP_MAX_ENUM);
//}
//
//inline VkImageLayout g_getVulkanImageLayout(EResourceLayout resourceLayout) {
//    static const std::unordered_map<EResourceLayout, VkImageLayout> imageLayoutMapping = {
//        {EResourceLayout::UNDEFINED,                          VK_IMAGE_LAYOUT_UNDEFINED},
//        {EResourceLayout::GENERAL,                            VK_IMAGE_LAYOUT_GENERAL},
//        {EResourceLayout::UAV,                                VK_IMAGE_LAYOUT_GENERAL},
//        {EResourceLayout::COLOR_ATTACHMENT,                   VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
//        {EResourceLayout::DEPTH_STENCIL_ATTACHMENT,           VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL},
//        {EResourceLayout::DEPTH_STENCIL_READ_ONLY,            VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL},
//        {EResourceLayout::SHADER_READ_ONLY,                   VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
//        {EResourceLayout::TRANSFER_SRC,                       VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL},
//        {EResourceLayout::TRANSFER_DST,                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL},
//        {EResourceLayout::PREINITIALIZED,                     VK_IMAGE_LAYOUT_PREINITIALIZED},
//        {EResourceLayout::DEPTH_READ_ONLY_STENCIL_ATTACHMENT, VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL},
//        {EResourceLayout::DEPTH_ATTACHMENT_STENCIL_READ_ONLY, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL},
//        {EResourceLayout::DEPTH_ATTACHMENT,                   VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL},
//        {EResourceLayout::DEPTH_READ_ONLY,                    VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL},
//        {EResourceLayout::STENCIL_ATTACHMENT,                 VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL},
//        {EResourceLayout::STENCIL_READ_ONLY,                  VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL},
//        {EResourceLayout::PRESENT_SRC,                        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR},
//        {EResourceLayout::SHARED_PRESENT,                     VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR},
//        {EResourceLayout::SHADING_RATE_NV,                    VK_IMAGE_LAYOUT_SHADING_RATE_OPTIMAL_NV},
//        {EResourceLayout::FRAGMENT_DENSITY_MAP_EXT,           VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT},
//        {EResourceLayout::READ_ONLY,                          VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL_KHR},
//        {EResourceLayout::ATTACHMENT,                         VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR},
//        {EResourceLayout::ACCELERATION_STRUCTURE,             VK_IMAGE_LAYOUT_GENERAL}
//    };
//
//    return getEnumMapping(resourceLayout, imageLayoutMapping, VK_IMAGE_LAYOUT_MAX_ENUM);
//}
//
//
//inline VkDynamicState g_getVulkanPipelineDynamicState(EPipelineDynamicState dynamicState) {
//    static const std::unordered_map<EPipelineDynamicState, VkDynamicState> dynamicStateMapping = {
//        {EPipelineDynamicState::VIEWPORT,                            VK_DYNAMIC_STATE_VIEWPORT},
//        {EPipelineDynamicState::SCISSOR,                             VK_DYNAMIC_STATE_SCISSOR},
//        {EPipelineDynamicState::LINE_WIDTH,                          VK_DYNAMIC_STATE_LINE_WIDTH},
//        {EPipelineDynamicState::DEPTH_BIAS,                          VK_DYNAMIC_STATE_DEPTH_BIAS},
//        {EPipelineDynamicState::BLEND_CONSTANTS,                     VK_DYNAMIC_STATE_BLEND_CONSTANTS},
//        {EPipelineDynamicState::DEPTH_BOUNDS,                        VK_DYNAMIC_STATE_DEPTH_BOUNDS},
//        {EPipelineDynamicState::STENCIL_COMPARE_MASK,                VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK},
//        {EPipelineDynamicState::STENCIL_WRITE_MASK,                  VK_DYNAMIC_STATE_STENCIL_WRITE_MASK},
//        {EPipelineDynamicState::STENCIL_REFERENCE,                   VK_DYNAMIC_STATE_STENCIL_REFERENCE},
//        {EPipelineDynamicState::CULL_MODE,                           VK_DYNAMIC_STATE_CULL_MODE},
//        {EPipelineDynamicState::FRONT_FACE,                          VK_DYNAMIC_STATE_FRONT_FACE},
//        {EPipelineDynamicState::PRIMITIVE_TOPOLOGY,                  VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY},
//        {EPipelineDynamicState::VIEWPORT_WITH_COUNT,                 VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT},
//        {EPipelineDynamicState::SCISSOR_WITH_COUNT,                  VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT},
//        {EPipelineDynamicState::VERTEX_INPUT_BINDING_STRIDE,         VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE},
//        {EPipelineDynamicState::DEPTH_TEST_ENABLE,                   VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE},
//        {EPipelineDynamicState::DEPTH_WRITE_ENABLE,                  VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE},
//        {EPipelineDynamicState::DEPTH_COMPARE_OP,                    VK_DYNAMIC_STATE_DEPTH_COMPARE_OP},
//        {EPipelineDynamicState::DEPTH_BOUNDS_TEST_ENABLE,            VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE},
//        {EPipelineDynamicState::STENCIL_TEST_ENABLE,                 VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE},
//        {EPipelineDynamicState::STENCIL_OP,                          VK_DYNAMIC_STATE_STENCIL_OP},
//        {EPipelineDynamicState::RASTERIZER_DISCARD_ENABLE,           VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE},
//        {EPipelineDynamicState::DEPTH_BIAS_ENABLE,                   VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE},
//        {EPipelineDynamicState::PRIMITIVE_RESTART_ENABLE,            VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE},
//        {EPipelineDynamicState::VIEWPORT_W_SCALING_NV,               VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_NV},
//        {EPipelineDynamicState::DISCARD_RECTANGLE_EXT,               VK_DYNAMIC_STATE_DISCARD_RECTANGLE_EXT},
//        {EPipelineDynamicState::SAMPLE_LOCATIONS_EXT,                VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT},
//        {EPipelineDynamicState::RAY_TRACING_PIPELINE_STACK_SIZE_KHR, VK_DYNAMIC_STATE_RAY_TRACING_PIPELINE_STACK_SIZE_KHR},
//        {EPipelineDynamicState::VIEWPORT_SHADING_RATE_PALETTE_NV,    VK_DYNAMIC_STATE_VIEWPORT_SHADING_RATE_PALETTE_NV},
//        {EPipelineDynamicState::VIEWPORT_COARSE_SAMPLE_ORDER_NV,     VK_DYNAMIC_STATE_VIEWPORT_COARSE_SAMPLE_ORDER_NV},
//        {EPipelineDynamicState::EXCLUSIVE_SCISSOR_NV,                VK_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_NV},
//        {EPipelineDynamicState::FRAGMENT_SHADING_RATE_KHR,           VK_DYNAMIC_STATE_FRAGMENT_SHADING_RATE_KHR},
//        {EPipelineDynamicState::LINE_STIPPLE_EXT,                    VK_DYNAMIC_STATE_LINE_STIPPLE_EXT},
//        {EPipelineDynamicState::VERTEX_INPUT_EXT,                    VK_DYNAMIC_STATE_VERTEX_INPUT_EXT},
//        {EPipelineDynamicState::PATCH_CONTROL_POINTS_EXT,            VK_DYNAMIC_STATE_PATCH_CONTROL_POINTS_EXT},
//        {EPipelineDynamicState::LOGIC_OP_EXT,                        VK_DYNAMIC_STATE_LOGIC_OP_EXT},
//        {EPipelineDynamicState::COLOR_WRITE_ENABLE_EXT,              VK_DYNAMIC_STATE_COLOR_WRITE_ENABLE_EXT}
//    };
//
//    return getEnumMapping(dynamicState, dynamicStateMapping, VK_DYNAMIC_STATE_MAX_ENUM);
//}

inline auto g_getVulkanMemoryPropertyFlagBits(EVulkanMemoryBits type) {
  switch (type) {
    case EVulkanMemoryBits::DEVICE_LOCAL:
      return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    case EVulkanMemoryBits::HOST_VISIBLE:
      return VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    case EVulkanMemoryBits::HOST_COHERENT:
      return VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    case EVulkanMemoryBits::HOST_CACHED:
      return VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
    case EVulkanMemoryBits::LAZILY_ALLOCATED:
      return VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;
    case EVulkanMemoryBits::PROTECTED:
      return VK_MEMORY_PROPERTY_PROTECTED_BIT;
    case EVulkanMemoryBits::DEVICE_COHERENT_AMD:
      return VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD;
    case EVulkanMemoryBits::DEVICE_UNCACHED_AMD:
      return VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD;
    case EVulkanMemoryBits::RDMA_CAPABLE_NV:
      return VK_MEMORY_PROPERTY_RDMA_CAPABLE_BIT_NV;
    case EVulkanMemoryBits::FLAG_BITS_MAX_ENUM:
      return VK_MEMORY_PROPERTY_FLAG_BITS_MAX_ENUM;
    default:
      break;
  }
  return static_cast<VkMemoryPropertyFlagBits>(type);
}

inline auto g_getVulkanBufferBits(EVulkanBufferBits type) {
  switch (type) {
    case EVulkanBufferBits::TRANSFER_SRC:
      return VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    case EVulkanBufferBits::TRANSFER_DST:
      return VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    case EVulkanBufferBits::UNIFORM_TEXEL_BUFFER:
      return VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
    case EVulkanBufferBits::STORAGE_TEXEL_BUFFER:
      return VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
    case EVulkanBufferBits::UNIFORM_BUFFER:
      return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    case EVulkanBufferBits::STORAGE_BUFFER:
      return VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    case EVulkanBufferBits::INDEX_BUFFER:
      return VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    case EVulkanBufferBits::VERTEX_BUFFER:
      return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    case EVulkanBufferBits::INDIRECT_BUFFER:
      return VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    case EVulkanBufferBits::SHADER_DEVICE_ADDRESS:
      return VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    case EVulkanBufferBits::TRANSFORM_FEEDBACK_BUFFER:
      return VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT;
    case EVulkanBufferBits::TRANSFORM_FEEDBACK_COUNTER_BUFFER:
      return VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT;
    case EVulkanBufferBits::CONDITIONAL_RENDERING:
      return VK_BUFFER_USAGE_CONDITIONAL_RENDERING_BIT_EXT;
    case EVulkanBufferBits::ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY:
      return VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
    case EVulkanBufferBits::ACCELERATION_STRUCTURE_STORAGE:
      return VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
    case EVulkanBufferBits::SHADER_BINDING_TABLE:
      return VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR;
    default:
      break;
  }
  return static_cast<VkBufferUsageFlagBits>(type);
}

inline VkColorComponentFlags g_getVulkanColorMask(EColorMask type) {
  VkColorComponentFlags result = 0;

  if (EColorMask::NONE == type) {
    return result;
  }

  if (EColorMask::ALL == type) {
    result = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
           | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  } else {
    if (!!(EColorMask::R & type)) {
      result |= VK_COLOR_COMPONENT_R_BIT;
    }
    if (!!(EColorMask::G & type)) {
      result |= VK_COLOR_COMPONENT_G_BIT;
    }
    if (!!(EColorMask::B & type)) {
      result |= VK_COLOR_COMPONENT_B_BIT;
    }
    if (!!(EColorMask::A & type)) {
      result |= VK_COLOR_COMPONENT_A_BIT;
    }
  }
  return result;
}

inline VkPipelineStageFlagBits g_getPipelineStageMask(EPipelineStageMask type) {
  switch (type) {
    case EPipelineStageMask::NONE:
      return VK_PIPELINE_STAGE_NONE;
    case EPipelineStageMask::TOP_OF_PIPE_BIT:
      return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    case EPipelineStageMask::DRAW_INDIRECT_BIT:
      return VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;
    case EPipelineStageMask::VERTEX_INPUT_BIT:
      return VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
    case EPipelineStageMask::VERTEX_SHADER_BIT:
      return VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
    case EPipelineStageMask::TESSELLATION_CONTROL_SHADER_BIT:
      return VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT;
    case EPipelineStageMask::TESSELLATION_EVALUATION_SHADER_BIT:
      return VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;
    case EPipelineStageMask::GEOMETRY_SHADER_BIT:
      return VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
    case EPipelineStageMask::FRAGMENT_SHADER_BIT:
      return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    case EPipelineStageMask::EARLY_FRAGMENT_TESTS_BIT:
      return VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    case EPipelineStageMask::LATE_FRAGMENT_TESTS_BIT:
      return VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    case EPipelineStageMask::COLOR_ATTACHMENT_OUTPUT_BIT:
      return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    case EPipelineStageMask::COMPUTE_SHADER_BIT:
      return VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    case EPipelineStageMask::TRANSFER_BIT:
      return VK_PIPELINE_STAGE_TRANSFER_BIT;
    case EPipelineStageMask::BOTTOM_OF_PIPE_BIT:
      return VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    case EPipelineStageMask::HOST_BIT:
      return VK_PIPELINE_STAGE_HOST_BIT;
    case EPipelineStageMask::ALL_GRAPHICS_BIT:
      return VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
    case EPipelineStageMask::ALL_COMMANDS_BIT:
      return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
  }

  assert(0);
  return VK_PIPELINE_STAGE_NONE;
}

inline VkDescriptorType g_getVulkanShaderBindingType(EShaderBindingType type) {
  switch (type) {
    case EShaderBindingType::UNIFORMBUFFER:
      return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    case EShaderBindingType::UNIFORMBUFFER_DYNAMIC:
      return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    case EShaderBindingType::TEXTURE_SAMPLER_SRV:
      return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    case EShaderBindingType::TEXTURE_SRV:
      return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    case EShaderBindingType::TEXTURE_UAV:
      return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    case EShaderBindingType::TEXTURE_ARRAY_SRV:
      return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    case EShaderBindingType::SAMPLER:
      return VK_DESCRIPTOR_TYPE_SAMPLER;
    case EShaderBindingType::BUFFER_SRV:
      return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    case EShaderBindingType::BUFFER_UAV:
      return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    case EShaderBindingType::BUFFER_UAV_DYNAMIC:
      return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
    case EShaderBindingType::BUFFER_TEXEL_SRV:
      return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
    case EShaderBindingType::BUFFER_TEXEL_UAV:
      return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
    case EShaderBindingType::ACCELERATION_STRUCTURE_SRV:
      return VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
    case EShaderBindingType::SUBPASS_INPUT_ATTACHMENT:
      return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    default:
      assert(0);
      break;
  }
  assert(0);
  return VK_DESCRIPTOR_TYPE_SAMPLER;
}

// clang-format on

}  // namespace game_engine

#endif  // GAME_ENGINE_RHI_TYPE_VK_H