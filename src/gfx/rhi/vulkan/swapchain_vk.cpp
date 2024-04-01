
#include "gfx/rhi/vulkan/swapchain_vk.h"

#include "gfx/rhi/vulkan/rhi_vk.h"

namespace game_engine {

// SwapchainImageVk
// =============================================
void SwapchainImageVk::ReleaseInternal() {
  m_texture_ = nullptr;
  if (Available) {
    g_rhi_vk->GetSemaphoreManager()->ReturnSemaphore(Available);
    Available = nullptr;
  }
  if (RenderFinished) {
    g_rhi_vk->GetSemaphoreManager()->ReturnSemaphore(RenderFinished);
    RenderFinished = nullptr;
  }
  if (RenderFinishedAfterShadow) {
    g_rhi_vk->GetSemaphoreManager()->ReturnSemaphore(RenderFinishedAfterShadow);
    RenderFinishedAfterShadow = nullptr;
  }
  if (RenderFinishedAfterBasePass) {
    g_rhi_vk->GetSemaphoreManager()->ReturnSemaphore(
        RenderFinishedAfterBasePass);
    RenderFinishedAfterBasePass = nullptr;
  }
}

// =============================================

// SwapchainVk
// =============================================

bool SwapchainVk::Create(const std::shared_ptr<Window>& window,
                         VkSurfaceKHR                   surface) {
  SwapChainSupportDetails swapChainSupport
      = QuerySwapChainSupport(g_rhi_vk->m_physicalDevice_, surface);

  // Choose the surface format
  VkSurfaceFormatKHR surfaceFormat{};
  if (!swapChainSupport.Formats.empty()) {
    surfaceFormat = swapChainSupport.Formats[0];
    for (const auto& availableFormat : swapChainSupport.Formats) {
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
      = ChooseSwapPresentMode(swapChainSupport.PresentModes, isVSyncEnabled);

  // Determine the swap extent
  VkExtent2D extent = swapChainSupport.Capabilities.currentExtent;
  if (extent.width == UINT32_MAX) {
    int width, height;
    SDL_GetWindowSize(window->getNativeWindowHandle(), &width, &height);

    extent.width
        = std::max(swapChainSupport.Capabilities.minImageExtent.width,
                   std::min(swapChainSupport.Capabilities.maxImageExtent.width,
                            static_cast<uint32_t>(width)));
    extent.height
        = std::max(swapChainSupport.Capabilities.minImageExtent.height,
                   std::min(swapChainSupport.Capabilities.maxImageExtent.height,
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
  uint32_t imageCount = swapChainSupport.Capabilities.minImageCount
                      + 1;  // enable at least double buffering
  if (swapChainSupport.Capabilities.maxImageCount > 0
      && imageCount > swapChainSupport.Capabilities.maxImageCount) {
    imageCount = swapChainSupport.Capabilities.maxImageCount;
  }

  VkSwapchainKHR oldSwapChain
      = m_swapChain_;  // use this instead of m_oldSwapChain_

  VkSwapchainCreateInfoKHR createInfo = {};
  createInfo.sType           = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface         = surface;
  createInfo.minImageCount   = imageCount;
  createInfo.imageFormat     = surfaceFormat.format;
  createInfo.imageColorSpace = surfaceFormat.colorSpace;
  createInfo.imageExtent     = extent;
  createInfo.imageArrayLayers
      = 1;  // Standard 2D Rendering (no stereoscopic 3D rendering)
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  QueueFamilyIndices indices
      = FindQueueFamilies(g_rhi_vk->m_physicalDevice_, surface);
  uint32_t queueFamilyIndices[]
      = {indices.GraphicsFamily.value(), indices.PresentFamily.value()};
  if (indices.GraphicsFamily != indices.PresentFamily) {
    createInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount = 2;  // Present + Graphics
    createInfo.pQueueFamilyIndices   = queueFamilyIndices;
  } else {
    createInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;        // optional
    createInfo.pQueueFamilyIndices   = nullptr;  // optional
  }

  createInfo.preTransform   = swapChainSupport.Capabilities.currentTransform;
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
    for (int32_t i = 0; i < (int32_t)Images.size(); i++) {
      SwapchainImageVk* SwapchainImage = Images[i];
      TextureVk* textureVk = (TextureVk*)SwapchainImage->m_texture_.get();
      if (textureVk->imageView) {
        vkDestroyImageView(g_rhi_vk->m_device_, textureVk->imageView, nullptr);
        textureVk->imageView = nullptr;
      }
      SwapchainImage->m_texture_.reset();
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

  Format = GetVulkanTextureFormat(surfaceFormat.format);
  Extent = math::Dimension2Di((int)extent.width, (int)extent.height);

  // ImageView
  Images.resize(swapChainImages.size());
  for (int32_t i = 0; i < Images.size(); ++i) {
    SwapchainImageVk* SwapchainImage = nullptr;
    if (oldSwapChain) {
      SwapchainImage = Images[i];
    } else {
      SwapchainImage = new SwapchainImageVk();
      SwapchainImage->Available
          = g_rhi_vk->GetSemaphoreManager()->GetOrCreateSemaphore();
      SwapchainImage->RenderFinished
          = g_rhi_vk->GetSemaphoreManager()->GetOrCreateSemaphore();
      SwapchainImage->RenderFinishedAfterShadow
          = g_rhi_vk->GetSemaphoreManager()->GetOrCreateSemaphore();
      SwapchainImage->RenderFinishedAfterBasePass
          = g_rhi_vk->GetSemaphoreManager()->GetOrCreateSemaphore();
      SwapchainImage->CommandBufferFence = nullptr;

      Images[i] = SwapchainImage;
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
    SwapchainImage->m_texture_ = std::make_shared<TextureVk>(
        ETextureType::TEXTURE_2D,
        GetVulkanTextureFormat(surfaceFormat.format),
        math::Dimension2Di{static_cast<int>(extent.width),
                           static_cast<int>(extent.height)});
    SwapchainImage->m_texture_->imageView = imageView;
    SwapchainImage->m_texture_->image     = swapChainImages[i];
    SwapchainImage->m_texture_->sampler   = textureSampler;
    SwapchainImage->m_texture_->mipLevels = 1; // TODO: temporal hot-fix
  }

  g_rhi_vk->RenderPassPool.Release();
  g_rhi_vk->PipelineStatePool.Release();

  for (DescriptorPoolVk* pool : g_rhi_vk->DescriptorPoolsSingleFrame) {
    pool->PendingDescriptorSets.clear();
    pool->AllocatedDescriptorSets.clear();
  }
  if (g_rhi_vk->DescriptorPoolMultiFrame) {
    g_rhi_vk->DescriptorPoolMultiFrame->PendingDescriptorSets.clear();
    g_rhi_vk->DescriptorPoolMultiFrame->AllocatedDescriptorSets.clear();
  }

  return true;
  // TODO: Clean up and recreate resources tied to the swap chain:
  // - Destroy and recreate framebuffers
  // - Destroy and recreate image views
  // - Recreate render pass if necessary
  // - Recreate graphics pipeline if necessary
  // - Re-record command buffers
  // - Recreate depth/stencil buffers if used
  // - Update/recreate descriptor sets if they reference swap chain resources
}

void SwapchainVk::ReleaseInternal() {
  vkDestroySwapchainKHR(g_rhi_vk->m_device_, m_swapChain_, nullptr);

  for (auto& iter : Images) {
    delete iter;
  }
}

// =============================================

}  // namespace game_engine
