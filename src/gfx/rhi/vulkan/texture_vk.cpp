#include "gfx/rhi/vulkan/rhi_vk.h"
#include "gfx/rhi/vulkan/utils_vk.h"

namespace game_engine {

void TextureVk::ReleaseInternal() {
  // If the image is created from a swapchain, the memory is nullptr, so there's
  // no need to destroy the image and memory. However, since the view is
  // directly created, it needs to be destroyed.
  if (deviceMemory) {
    if (image) {
      vkDestroyImage(g_rhi_vk->m_device_, image, nullptr);
    }

    vkFreeMemory(g_rhi_vk->m_device_, deviceMemory, nullptr);
  }

  if (imageView) {
    vkDestroyImageView(g_rhi_vk->m_device_, imageView, nullptr);
  }
}

uint32_t TextureVk::GetMipLevels(uint32_t InWidth, uint32_t InHeight) {
  return 1
       + static_cast<uint32_t>(
             std::floor(std::log2(std::max(InWidth, InHeight))));
}

VkSampler TextureVk::CreateDefaultSamplerState() {
  if (g_defaultSampler) {
    return g_defaultSampler;
  }

  VkSamplerCreateInfo samplerInfo = {};
  samplerInfo.sType               = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  samplerInfo.magFilter           = VK_FILTER_LINEAR;
  samplerInfo.minFilter           = VK_FILTER_LINEAR;

  samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.borderColor  = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

  samplerInfo.anisotropyEnable = VK_TRUE;
  samplerInfo.maxAnisotropy    = 16;

  samplerInfo.unnormalizedCoordinates = VK_FALSE;

  samplerInfo.compareEnable = VK_FALSE;
  samplerInfo.compareOp     = VK_COMPARE_OP_ALWAYS;

  // No upper bound for max mip level
  // TODO: remove code
  /*const uint32_t MipLevels
      = static_cast<uint32_t>(
            std::floor(std::log2(std::max<int>(SCR_WIDTH, SCR_HEIGHT))))
      + 1;*/

  samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  samplerInfo.mipLodBias = 0.0f;  // Optional
  samplerInfo.minLod     = 0.0f;  // Optional
  samplerInfo.maxLod     = FLT_MAX;

  if (vkCreateSampler(
          g_rhi_vk->m_device_, &samplerInfo, nullptr, &g_defaultSampler)
      != VK_SUCCESS) {
    GlobalLogger::Log(LogLevel::Error, "Failed to create default sampler");
    return nullptr;
  }

  return g_defaultSampler;
}

}  // namespace game_engine
