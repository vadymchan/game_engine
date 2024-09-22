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

inline auto g_getVulkanTextureComponentCount(ETextureFormat type) {
    static const std::unordered_map<ETextureFormat, int> textureComponentCountMapping = {
        {ETextureFormat::RGB8,       3},
        {ETextureFormat::RGB32F,     3},
        {ETextureFormat::RGB16F,     3},
        {ETextureFormat::R11G11B10F, 3},

        {ETextureFormat::RGBA8,      4},
        {ETextureFormat::RGBA16F,    4},
        {ETextureFormat::RGBA32F,    4},
        {ETextureFormat::RGBA8SI,    4},
        {ETextureFormat::RGBA8UI,    4},

        {ETextureFormat::BGRA8,      4},

        {ETextureFormat::R8,         1},
        {ETextureFormat::R16F,       1},
        {ETextureFormat::R32F,       1},
        {ETextureFormat::R8UI,       1},
        {ETextureFormat::R32UI,      1},

        {ETextureFormat::RG8,        2},
        {ETextureFormat::RG16F,      2},
        {ETextureFormat::RG32F,      2},

        {ETextureFormat::D16,        1},
        {ETextureFormat::D16_S8,     2},
        {ETextureFormat::D24,        1},
        {ETextureFormat::D24_S8,     2},
        {ETextureFormat::D32,        1},
        {ETextureFormat::D32_S8,     2}
    };

    return getEnumMapping(textureComponentCountMapping, type, 0);  
}

inline auto g_getVulkanTexturePixelSize(ETextureFormat type) {
    static const std::unordered_map<ETextureFormat, int> texturePixelSizeMapping = {
        {ETextureFormat::RGB8,       3},
        {ETextureFormat::RGB32F,     12},
        {ETextureFormat::RGB16F,     6},
        {ETextureFormat::R11G11B10F, 4},

        {ETextureFormat::RGBA8,      4},
        {ETextureFormat::RGBA16F,    8},
        {ETextureFormat::RGBA32F,    16},
        {ETextureFormat::RGBA8SI,    4},
        {ETextureFormat::RGBA8UI,    4},

        {ETextureFormat::BGRA8,      4},

        {ETextureFormat::R8,         1},
        {ETextureFormat::R16F,       2},
        {ETextureFormat::R32F,       4},
        {ETextureFormat::R8UI,       1},
        {ETextureFormat::R32UI,      4},

        {ETextureFormat::RG8,        2},
        {ETextureFormat::RG16F,      2},
        {ETextureFormat::RG32F,      4},

        {ETextureFormat::D16,        2},
        {ETextureFormat::D16_S8,     3},
        {ETextureFormat::D24,        3},
        {ETextureFormat::D24_S8,     4},
        {ETextureFormat::D32,        4},
        {ETextureFormat::D32_S8,     5}
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

VkPrimitiveTopology g_getVulkanPrimitiveTopology(EPrimitiveType primitiveType);
EPrimitiveType g_getVulkanPrimitiveTopology(VkPrimitiveTopology primitiveTopology);

VkVertexInputRate g_getVulkanVertexInputRate(EVertexInputRate inputRate);
EVertexInputRate g_getVulkanVertexInputRate(VkVertexInputRate inputRate);

VkPolygonMode g_getVulkanPolygonMode(EPolygonMode polygonMode);
EPolygonMode g_getVulkanPolygonMode(VkPolygonMode polygonMode);

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
