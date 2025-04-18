#ifndef GAME_ENGINE_RHI_ENUMS_VK_H
#define GAME_ENGINE_RHI_ENUMS_VK_H

#include "gfx/rhi/rhi_new/common/rhi_enums.h"

#include <vulkan/vulkan.h>

#include <unordered_map>

namespace game_engine {
namespace gfx {
namespace rhi {

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

VkColorComponentFlags g_getColorMaskVk(ColorMask type);

// Descriptor types
VkDescriptorType g_getShaderBindingTypeVk(ShaderBindingType type);

}  // namespace rhi
}  // namespace gfx
}  // namespace game_engine

#endif  // GAME_ENGINE_RHI_ENUMS_VK_H