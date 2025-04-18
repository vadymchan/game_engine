#ifndef GAME_ENGINE_RHI_ENUMS_VK_H
#define GAME_ENGINE_RHI_ENUMS_VK_H

#include "gfx/rhi/rhi_new/common/rhi_enums.h"

#include <vulkan/vulkan.h>

#include <unordered_map>

namespace game_engine {
namespace gfx {
namespace rhi {

// TODO: if there's a need, add bitwise operations to this enum
enum class VulkanBufferBits : uint32_t {
  TransferSrc                             = 0x00'00'00'01,
  TransferDst                             = 0x00'00'00'02,
  UniformTexelBuffer                      = 0x00'00'00'04,
  StorageTexelBuffer                      = 0x00'00'00'08,
  UniformBuffer                           = 0x00'00'00'10,
  StorageBuffer                           = 0x00'00'00'20,
  IndexBuffer                             = 0x00'00'00'40,
  VertexBuffer                            = 0x00'00'00'80,
  IndirectBuffer                          = 0x00'00'01'00,
  ShaderDeviceAddress                     = 0x00'02'00'00,
  TransformFeedbackBuffer                 = 0x00'00'08'00,
  TransformFeedbackCounterBuffer          = 0x00'00'10'00,
  ConditionalRendering                    = 0x00'00'02'00,
  AccelerationStructureBuildInputReadOnly = 0x00'08'00'00,
  AccelerationStructureStorage            = 0x00'10'00'00,
  ShaderBindingTable                      = 0x00'00'04'00,
};

// TODO: if there's a need, add bitwise operations to this enum
enum class VulkanMemoryBits : uint32_t {
  DeviceLocal       = 0x00'00'00'01,
  HostVisible       = 0x00'00'00'02,
  HostCoherent      = 0x00'00'00'04,
  HostCached        = 0x00'00'00'08,
  LazilyAllocated   = 0x00'00'00'10,
  Protected         = 0x00'00'00'20,
  DeviceCoherentAmd = 0x00'00'00'40,
  DeviceUncachedAmd = 0x00'00'00'80,
  RdmaCapableNv     = 0x00'00'01'00,
  FlagBitsMaxEnum   = 0x7F'FF'FF'FF
};


VkFilter      g_getTextureFilterTypeVk(TextureFilter textureFilter);
TextureFilter g_getTextureFilterTypeVk(VkFilter filter);

VkSamplerAddressMode g_getTextureAddressModeVk(TextureAddressMode addressMode);
TextureAddressMode   g_getTextureAddressModeVk(VkSamplerAddressMode addressMode);

VkSamplerMipmapMode g_getTextureMipmapModeVk(TextureFilter textureFilter);
TextureFilter       g_getTextureMipmapModeVk(VkSamplerMipmapMode mipmapMode);

VkFormat      g_getTextureFormatVk(TextureFormat textureFormat);
TextureFormat g_getTextureFormatVk(VkFormat format);

int g_getTextureComponentCountVk(TextureFormat type);
int g_getTexturePixelSizeVk(TextureFormat type);

void g_getAttachmentLoadStoreOpVk(VkAttachmentLoadOp&   loadOp,
                                      VkAttachmentStoreOp&  storeOp,
                                      AttachmentLoadStoreOp type);

VkShaderStageFlagBits g_getShaderStageFlagVk(ShaderStageFlag type);

VkPrimitiveTopology g_getPrimitiveTopologyVk(PrimitiveType primitiveType);
PrimitiveType       g_getPrimitiveTopologyVk(VkPrimitiveTopology primitiveTopology);

VkVertexInputRate g_getVertexInputRateVk(VertexInputRate inputRate);
VertexInputRate   g_getVertexInputRateVk(VkVertexInputRate inputRate);

VkPolygonMode g_getPolygonModeVk(PolygonMode polygonMode);
PolygonMode   g_getPolygonModeVk(VkPolygonMode polygonMode);

VkCullModeFlags g_getCullModeVk(CullMode cullMode);

VkFrontFace g_getFrontFaceVk(FrontFace frontFace);
FrontFace   g_getFrontFaceVk(VkFrontFace frontFace);

VkStencilOp g_getStencilOpVk(StencilOp stencilOp);
StencilOp   g_getStencilOpVk(VkStencilOp stencilOp);

VkCompareOp g_getCompareOpVk(CompareOp compareOp);
CompareOp   g_getCompareOpVk(VkCompareOp compareOp);

VkBlendFactor g_getBlendFactorVk(BlendFactor blendFactor);
BlendFactor   g_getBlendFactorVk(VkBlendFactor blendFactor);

VkBlendOp g_getBlendOpVk(BlendOp blendOp);
BlendOp   g_getBlendOpVk(VkBlendOp blendOp);

VkImageLayout  g_getImageLayoutVk(ResourceLayout resourceLayout);
ResourceLayout g_getImageLayoutVk(VkImageLayout resourceLayout);

VkMemoryPropertyFlagBits g_getMemoryPropertyFlagBitsVk(VulkanMemoryBits type);
VkBufferUsageFlagBits    g_getBufferBitsVk(VulkanBufferBits type);

VkColorComponentFlags g_getColorMaskVk(ColorMask type);

VkPipelineStageFlagBits g_getPipelineStageMaskVk(PipelineStageMask type);

// Descriptor types
VkDescriptorType g_getShaderBindingTypeVk(ShaderBindingType type);

}  // namespace rhi
}  // namespace gfx
}  // namespace game_engine

#endif  // GAME_ENGINE_RHI_ENUMS_VK_H