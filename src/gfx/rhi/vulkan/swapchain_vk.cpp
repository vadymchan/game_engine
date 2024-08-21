
#include "gfx/rhi/vulkan/swapchain_vk.h"

#include "gfx/rhi/vulkan/rhi_vk.h"

namespace game_engine {

// SwapchainImageVk
// =============================================
void SwapchainImageVk::releaseInternal() {
  m_TexturePtr_ = nullptr;
  if (m_available_) {
    g_rhi_vk->getSemaphoreManager()->returnSemaphore(m_available_);
    m_available_ = nullptr;
  }
  if (m_renderFinished_) {
    g_rhi_vk->getSemaphoreManager()->returnSemaphore(m_renderFinished_);
    m_renderFinished_ = nullptr;
  }
  if (m_renderFinishedAfterShadow_) {
    g_rhi_vk->getSemaphoreManager()->returnSemaphore(m_renderFinishedAfterShadow_);
    m_renderFinishedAfterShadow_ = nullptr;
  }
  if (m_renderFinishedAfterBasePass_) {
    g_rhi_vk->getSemaphoreManager()->returnSemaphore(
        m_renderFinishedAfterBasePass_);
    m_renderFinishedAfterBasePass_ = nullptr;
  }
}

// =============================================

// SwapchainVk
// =============================================

bool SwapchainVk::create(const std::shared_ptr<Window>& window) {
  SwapChainSupportDetails swapChainSupport = g_querySwapChainSupport(
      g_rhi_vk->m_physicalDevice_, g_rhi_vk->m_surface_);

  // Choose the surface format
  VkSurfaceFormatKHR surfaceFormat{};
  if (!swapChainSupport.m_formats_.empty()) {
    surfaceFormat = swapChainSupport.m_formats_[0];
    for (const auto& availableFormat : swapChainSupport.m_formats_) {
      if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM
          && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
        surfaceFormat = availableFormat;
        break;
      }
    }
  } else {
    GlobalLogger::Log(LogLevel::Error, "No valid surface formats found");
    return false;
  }

  // Choose the present mode
  VkPresentModeKHR presentMode
      = g_chooseSwapPresentMode(swapChainSupport.m_presentModes_, m_isVSyncEnabled_);

  // Determine the swap extent
  VkExtent2D extent = swapChainSupport.m_capabilities_.currentExtent;
  if (extent.width == UINT32_MAX) {
    int width, height;
    SDL_GetWindowSize(window->getNativeWindowHandle(), &width, &height);

    extent.width
        = std::max(swapChainSupport.m_capabilities_.minImageExtent.width,
                   std::min(swapChainSupport.m_capabilities_.maxImageExtent.width,
                            static_cast<uint32_t>(width)));
    extent.height
        = std::max(swapChainSupport.m_capabilities_.minImageExtent.height,
                   std::min(swapChainSupport.m_capabilities_.maxImageExtent.height,
                            static_cast<uint32_t>(height)));

    // TODO: use this code instead of code above
    // extent.width
    //    = std::clamp(static_cast<uint32_t>(width),
    //                 swapChainSupport.Capabilities.minImageExtent.width,
    //                 swapChainSupport.Capabilities.maxImageExtent.width);

    // extent.height
    //     = std::clamp(static_cast<uint32_t>(height),
    //                  swapChainSupport.Capabilities.minImageExtent.height,
    //                  swapChainSupport.Capabilities.maxImageExtent.height);
  }

  // Specify the number of images in the swapchain
  uint32_t imageCount = swapChainSupport.m_capabilities_.minImageCount
                      + 1;  // enable at least double buffering
  if (swapChainSupport.m_capabilities_.maxImageCount > 0
      && imageCount > swapChainSupport.m_capabilities_.maxImageCount) {
    imageCount = swapChainSupport.m_capabilities_.maxImageCount;
  }

  VkSwapchainKHR oldSwapChain
      = m_swapChain_;  // use this instead of m_oldSwapChain_

  VkSwapchainCreateInfoKHR createInfo = {};
  createInfo.sType           = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface         = g_rhi_vk->m_surface_;
  createInfo.minImageCount   = imageCount;
  createInfo.imageFormat     = surfaceFormat.format;
  createInfo.imageColorSpace = surfaceFormat.colorSpace;
  createInfo.imageExtent     = extent;
  createInfo.imageArrayLayers
      = 1;  // Standard 2D Rendering (no stereoscopic 3D rendering)
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  QueueFamilyIndices indices
      = g_findQueueFamilies(g_rhi_vk->m_physicalDevice_, g_rhi_vk->m_surface_);
  uint32_t queueFamilyIndices[]
      = {indices.m_graphicsFamily_.value(), indices.m_presentFamily_.value()};
  if (indices.m_graphicsFamily_ != indices.m_presentFamily_) {
    createInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount = 2;  // Present + Graphics
    createInfo.pQueueFamilyIndices   = queueFamilyIndices;
  } else {
    createInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;        // optional
    createInfo.pQueueFamilyIndices   = nullptr;  // optional
  }

  createInfo.preTransform   = swapChainSupport.m_capabilities_.currentTransform;
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  createInfo.presentMode    = presentMode;
  createInfo.clipped        = VK_TRUE;
  createInfo.oldSwapchain   = oldSwapChain;  // TODO : add old swapchain

  if (vkCreateSwapchainKHR(
          g_rhi_vk->m_device_, &createInfo, nullptr, &m_swapChain_)
      != VK_SUCCESS) {
    GlobalLogger::Log(LogLevel::Error, "Failed to create swapchain");
    return false;
  }

  // Destroy old swapchain
  if (oldSwapChain != VK_NULL_HANDLE) {
    // TODO: Destroy old image views
    for (int32_t i = 0; i < (int32_t)m_images_.size(); i++) {
      SwapchainImageVk* swapchainImage = m_images_[i];
      TextureVk* textureVk = (TextureVk*)swapchainImage->m_TexturePtr_.get();
      if (textureVk->m_imageView_) {
        vkDestroyImageView(g_rhi_vk->m_device_, textureVk->m_imageView_, nullptr);
        textureVk->m_imageView_ = nullptr;
      }
      swapchainImage->m_TexturePtr_.reset();
      // delete Images[i];
    }
    vkDestroySwapchainKHR(g_rhi_vk->m_device_, oldSwapChain, nullptr);
  }

  // Retrieve swapchain images
  vkGetSwapchainImagesKHR(
      g_rhi_vk->m_device_, m_swapChain_, &imageCount, nullptr);
  std::vector<VkImage> swapChainImages(imageCount);
  vkGetSwapchainImagesKHR(
      g_rhi_vk->m_device_, m_swapChain_, &imageCount, swapChainImages.data());

  m_format_ = g_getVulkanTextureFormat(surfaceFormat.format);
  m_extent_ = math::Dimension2Di((int)extent.width, (int)extent.height);

  // ImageView
  m_images_.resize(swapChainImages.size());
  for (int32_t i = 0; i < m_images_.size(); ++i) {
    SwapchainImageVk* swapchainImage = nullptr;
    if (oldSwapChain) {
      swapchainImage = m_images_[i];
    } else {
      swapchainImage = new SwapchainImageVk();
      swapchainImage->m_available_
          = g_rhi_vk->getSemaphoreManager()->getOrCreateSemaphore();
      swapchainImage->m_renderFinished_
          = g_rhi_vk->getSemaphoreManager()->getOrCreateSemaphore();
      swapchainImage->m_renderFinishedAfterShadow_
          = g_rhi_vk->getSemaphoreManager()->getOrCreateSemaphore();
      swapchainImage->m_renderFinishedAfterBasePass_
          = g_rhi_vk->getSemaphoreManager()->getOrCreateSemaphore();
      swapchainImage->m_commandBufferFence_ = nullptr;

      m_images_[i] = swapchainImage;
    }

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image    = swapChainImages[i];
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format   = surfaceFormat.format;
    viewInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel   = 0;
    viewInfo.subresourceRange.levelCount     = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount     = 1;
    viewInfo.components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY;

    VkImageView imageView;
    if (vkCreateImageView(g_rhi_vk->m_device_, &viewInfo, nullptr, &imageView)
        != VK_SUCCESS) {
      GlobalLogger::Log(LogLevel::Error, "Failed to create image view");
      // TODO: Cleanup and return false
    }

    // TODO: consider remove sampler from swapchain image (it's nullptr in
    // swapchain image)
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter               = VK_FILTER_LINEAR;
    samplerInfo.minFilter               = VK_FILTER_LINEAR;
    samplerInfo.addressModeU            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.anisotropyEnable        = VK_TRUE;
    samplerInfo.maxAnisotropy           = 16;
    samplerInfo.borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable           = VK_FALSE;
    samplerInfo.compareOp               = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias              = 0.0f;
    samplerInfo.minLod                  = 0.0f;
    samplerInfo.maxLod                  = 0.0f;

    VkSampler textureSampler;
    if (vkCreateSampler(
            g_rhi_vk->m_device_, &samplerInfo, nullptr, &textureSampler)
        != VK_SUCCESS) {
      GlobalLogger::Log(LogLevel::Error, "Failed to create texture sampler");
      // TODO: Cleanup and return false
    }

    // Create TextureVk object with the new image view

    swapchainImage->m_TexturePtr_ = std::make_shared<TextureVk>(
        ETextureType::TEXTURE_2D,
        g_getVulkanTextureFormat(surfaceFormat.format),
        math::Dimension2Di{static_cast<int>(extent.width),
                           static_cast<int>(extent.height)});

    std::shared_ptr<TextureVk> swapchainImageTextureVk
        = std::static_pointer_cast<TextureVk>(swapchainImage->m_TexturePtr_);

    swapchainImageTextureVk->m_imageView_ = imageView;
    swapchainImageTextureVk->m_image_     = swapChainImages[i];
    swapchainImageTextureVk->m_sampler_   = textureSampler;
    swapchainImageTextureVk->m_mipLevels_ = 1;  // TODO: temporal hot-fix
  }

  g_rhi_vk->s_renderPassPool.release();
  g_rhi_vk->s_pipelineStatePool.release();

  for (DescriptorPoolVk* pool : g_rhi_vk->m_descriptorPoolsSingleFrame_) {
    pool->m_pendingDescriptorSets_.clear();
    pool->m_allocatedDescriptorSets_.clear();
  }
  if (g_rhi_vk->m_descriptorPoolMultiFrame_) {
    g_rhi_vk->m_descriptorPoolMultiFrame_->m_pendingDescriptorSets_.clear();
    g_rhi_vk->m_descriptorPoolMultiFrame_->m_allocatedDescriptorSets_.clear();
  }

  return true;
  // TODO: Clean up and recreate resources tied to the swap chain:
  // - Destroy and recreate framebuffers
  // - Destroy and recreate image views
  // - Recreate render pass if necessary
  // - Recreate graphics pipeline if necessary
  // - Re-record command buffers
  // - Recreate depth/stencil buffers if used
  // - update/recreate descriptor sets if they reference swap chain resources
}

void SwapchainVk::releaseInternal() {
  vkDestroySwapchainKHR(g_rhi_vk->m_device_, m_swapChain_, nullptr);

  for (auto& iter : m_images_) {
    delete iter;
  }
}

// =============================================

}  // namespace game_engine
