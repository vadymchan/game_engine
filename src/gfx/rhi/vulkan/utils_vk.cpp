
#include "gfx/rhi/vulkan/utils_vk.h"

#include "gfx/rhi/vulkan/rhi_vk.h"
// TODO: ensure that no circular dependencies are created
#include "gfx/rhi/vulkan/buffer_vk.h"
#include "gfx/rhi/vulkan/memory_pool_vk.h"
#include "gfx/rhi/vulkan/texture_vk.h"

#include <iostream>

namespace game_engine {

// callback function for Vulkan debug messages
VKAPI_ATTR VkBool32 VKAPI_CALL
    g_debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
                    VkDebugUtilsMessageTypeFlagsEXT             messageType,
                    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                    void*                                       pUserData) {
  // message severity is at least a warning
  if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
    // TODO: implement some logic
  }

  GlobalLogger::Log(
      LogLevel::Debug,
      "validation layer: " + std::string(pCallbackData->pMessage));

  return VK_FALSE;
}

SwapChainSupportDetails g_querySwapChainSupport(VkPhysicalDevice device,
                                                VkSurfaceKHR     surface) {
  SwapChainSupportDetails details;

  // Retrieve the basic surface capabilities.
  VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
      device, surface, &details.m_capabilities_);
  if (result != VK_SUCCESS) {
    GlobalLogger::Log(LogLevel::Error,
                      "Failed to get physical device surface capabilities");
  }

  // Retrieve the supported surface formats.
  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
  if (formatCount != 0) {
    details.m_formats_.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(
        device, surface, &formatCount, details.m_formats_.data());
  }

  // Retrieve the supported presentation modes.
  uint32_t presentModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(
      device, surface, &presentModeCount, nullptr);
  if (presentModeCount != 0) {
    details.m_presentModes_.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        device, surface, &presentModeCount, details.m_presentModes_.data());
  }

  // Future expansions:
  // add additional checks or preferences here, such as prioritizing
  // certain formats or present modes based on performance or specific
  // requirements of engine.

  return details;
}

VkPresentModeKHR g_chooseSwapPresentMode(
    const std::vector<VkPresentModeKHR>& availablePresentModes,
    bool                                 m_isVSyncEnabled_) {
  if (m_isVSyncEnabled_) {
    // TODO: consider checking that physical device supports vsync, not only
    // parameter set to function
    return VK_PRESENT_MODE_FIFO_KHR;
  }

  for (const auto& availablePresentMode : availablePresentModes) {
    if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
      return availablePresentMode;
    }
  }

  return VK_PRESENT_MODE_FIFO_KHR;
}

QueueFamilyIndices g_findQueueFamilies(VkPhysicalDevice device,
                                       VkSurfaceKHR     surface) {
  QueueFamilyIndices indices;

  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(
      device, &queueFamilyCount, queueFamilies.data());

  for (uint32_t    queueFamilyIndex = 0;
       const auto& queueFamilyProperties : queueFamilies) {
    // Check for Graphics Queue Support
    if (queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      indices.m_graphicsFamily_ = queueFamilyIndex;
    }

    // Check for Presentation Support
    VkBool32 supportsPresent = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(
        device, queueFamilyIndex, surface, &supportsPresent);
    if (supportsPresent) {
      indices.m_presentFamily_ = queueFamilyIndex;
    }

    // Check for Compute Queue Support
    if (queueFamilyProperties.queueFlags & VK_QUEUE_COMPUTE_BIT) {
      indices.m_computeFamily_ = queueFamilyIndex;
    }

    if (indices.isComplete()) {
      break;
    }

    ++queueFamilyIndex;
  }

  return indices;
}

VkSemaphore g_createSemaphore() {
  VkSemaphoreCreateInfo semaphoreInfo{};
  semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkSemaphore semaphore;
  if (vkCreateSemaphore(g_rhiVk->m_device_, &semaphoreInfo, nullptr, &semaphore)
      != VK_SUCCESS) {
    GlobalLogger::Log(LogLevel::Error, "Failed to create semaphore");
  }

  return semaphore;
}

VkFence g_createFence(bool signaled) {
  VkFenceCreateInfo fenceInfo{};
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceInfo.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT
                             : 0;  // Optionally create it in a signaled state

  VkFence fence;
  if (vkCreateFence(g_rhiVk->m_device_, &fenceInfo, nullptr, &fence)
      != VK_SUCCESS) {
    GlobalLogger::Log(LogLevel::Error, "Failed to create fence");
  }

  return fence;
}

// =============== m_buffer Utils ===============

// --------------- image (texture) ------------

bool g_isDepthFormat(VkFormat format) {
  switch (format) {
    case VK_FORMAT_D16_UNORM:
    case VK_FORMAT_X8_D24_UNORM_PACK32:
    case VK_FORMAT_D32_SFLOAT:
    case VK_FORMAT_D16_UNORM_S8_UINT:
    case VK_FORMAT_D24_UNORM_S8_UINT:
    case VK_FORMAT_D32_SFLOAT_S8_UINT:
      return true;
    default:
      return false;
  }
}

bool g_isDepthOnlyFormat(VkFormat format) {
  switch (format) {
    case VK_FORMAT_D16_UNORM:
    case VK_FORMAT_X8_D24_UNORM_PACK32:
    case VK_FORMAT_D32_SFLOAT:
      return true;
  }

  return false;
}

bool g_isTextureCube(VkImageViewType    imageViewType,
                     uint32_t           layerCount,
                     VkImageCreateFlags flags) {
  return imageViewType == VK_IMAGE_VIEW_TYPE_CUBE
      || imageViewType == VK_IMAGE_VIEW_TYPE_CUBE_ARRAY
             && (flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT);
}

bool g_isTexture2DArray(VkImageViewType    imageViewType,
                        uint32_t           layerCount,
                        VkImageCreateFlags flags) {
  return (imageViewType == VK_IMAGE_VIEW_TYPE_2D_ARRAY) && layerCount > 1
      && !(flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT);
}

VkImageView g_createTextureView(VkImage            image,
                                VkFormat           format,
                                VkImageAspectFlags aspectMask,
                                uint32_t           mipLevels) {
  VkImageViewCreateInfo viewInfo = {};

  viewInfo.sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.image    = image;
  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewInfo.format   = format;

  viewInfo.subresourceRange.aspectMask     = aspectMask;
  viewInfo.subresourceRange.baseMipLevel   = 0;
  viewInfo.subresourceRange.levelCount     = mipLevels;
  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.subresourceRange.layerCount     = 1;

  viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
  viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
  viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
  viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

  VkImageView imageView;
  if (vkCreateImageView(g_rhiVk->m_device_, &viewInfo, nullptr, &imageView)
      != VK_SUCCESS) {
    GlobalLogger::Log(LogLevel::Error, "Failed to create image view");
  }

  return imageView;
}

VkImageView g_createTexture2DArrayView(VkImage            image,
                                       uint32_t           layerCount,
                                       VkFormat           format,
                                       VkImageAspectFlags aspectMask,
                                       uint32_t           mipLevels) {
  VkImageViewCreateInfo viewInfo = {};

  viewInfo.sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.image    = image;
  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
  viewInfo.format   = format;

  viewInfo.subresourceRange.aspectMask     = aspectMask;
  viewInfo.subresourceRange.baseMipLevel   = 0;
  viewInfo.subresourceRange.levelCount     = mipLevels;
  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.subresourceRange.layerCount     = layerCount;

  viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
  viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
  viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
  viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

  VkImageView imageView;
  if (vkCreateImageView(g_rhiVk->m_device_, &viewInfo, nullptr, &imageView)
      != VK_SUCCESS) {
    GlobalLogger::Log(LogLevel::Error, "Failed to create 2D array image view");
  }

  return imageView;
}

VkImageView g_createTextureCubeView(VkImage            image,
                                    VkFormat           format,
                                    VkImageAspectFlags aspectMask,
                                    uint32_t           mipLevels) {
  VkImageViewCreateInfo viewInfo = {};

  viewInfo.sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.image    = image;
  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
  viewInfo.format   = format;

  viewInfo.subresourceRange.aspectMask     = aspectMask;
  viewInfo.subresourceRange.baseMipLevel   = 0;
  viewInfo.subresourceRange.levelCount     = mipLevels;
  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.subresourceRange.layerCount     = 6;

  viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
  viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
  viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
  viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

  VkImageView imageView;
  if (vkCreateImageView(g_rhiVk->m_device_, &viewInfo, nullptr, &imageView)
      != VK_SUCCESS) {
    GlobalLogger::Log(LogLevel::Error, "Failed to create cube image view");
  }

  return imageView;
}

VkImageView g_createTextureViewForSpecificMipMap(VkImage            image,
                                                 VkFormat           format,
                                                 VkImageAspectFlags aspectMask,
                                                 uint32_t mipLevelIndex) {
  VkImageViewCreateInfo viewInfo = {};

  viewInfo.sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.image    = image;
  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewInfo.format   = format;

  viewInfo.subresourceRange.aspectMask     = aspectMask;
  viewInfo.subresourceRange.baseMipLevel   = mipLevelIndex;
  viewInfo.subresourceRange.levelCount     = 1;
  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.subresourceRange.layerCount     = 1;

  viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
  viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
  viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
  viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

  VkImageView imageView;
  if (vkCreateImageView(g_rhiVk->m_device_, &viewInfo, nullptr, &imageView)
      != VK_SUCCESS) {
    GlobalLogger::Log(LogLevel::Error,
                      "Failed to create image view for specific mip level");
  }

  return imageView;
}

VkImageView g_createTexture2DArrayViewForSpecificMipMap(
    VkImage            image,
    uint32_t           layerCount,
    VkFormat           format,
    VkImageAspectFlags aspectMask,
    uint32_t           mipLevelIndex) {
  VkImageViewCreateInfo viewInfo = {};

  viewInfo.sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.image    = image;
  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
  viewInfo.format   = format;

  viewInfo.subresourceRange.aspectMask     = aspectMask;
  viewInfo.subresourceRange.baseMipLevel   = mipLevelIndex;
  viewInfo.subresourceRange.levelCount     = 1;
  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.subresourceRange.layerCount     = layerCount;

  viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
  viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
  viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
  viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

  VkImageView imageView;
  if (vkCreateImageView(g_rhiVk->m_device_, &viewInfo, nullptr, &imageView)
      != VK_SUCCESS) {
    GlobalLogger::Log(
        LogLevel::Error,
        "Failed to create 2D array image view for specific mip level");
  }

  return imageView;
}

VkImageView g_createTextureCubeViewForSpecificMipMap(
    VkImage            image,
    VkFormat           format,
    VkImageAspectFlags aspectMask,
    uint32_t           mipLevelIndex) {
  VkImageViewCreateInfo viewInfo = {};

  viewInfo.sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.image    = image;
  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
  viewInfo.format   = format;

  viewInfo.subresourceRange.aspectMask     = aspectMask;
  viewInfo.subresourceRange.baseMipLevel   = mipLevelIndex;
  viewInfo.subresourceRange.levelCount     = 1;
  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.subresourceRange.layerCount     = 6;

  viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
  viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
  viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
  viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

  VkImageView imageView;
  if (vkCreateImageView(g_rhiVk->m_device_, &viewInfo, nullptr, &imageView)
      != VK_SUCCESS) {
    GlobalLogger::Log(
        LogLevel::Error,
        "Failed to create cube image view for specific mip level");
  }

  return imageView;
}

bool g_createTexture2DArrayLowLevel(uint32_t              width,
                                    uint32_t              height,
                                    uint32_t              arrayLayers,
                                    uint32_t              mipLevels,
                                    VkSampleCountFlagBits numSamples,
                                    VkFormat              format,
                                    VkImageTiling         tiling,
                                    VkImageUsageFlags     usage,
                                    VkMemoryPropertyFlags properties,
                                    VkImageLayout         imageLayout,
                                    VkImageCreateFlagBits imageCreateFlagBits,
                                    VkImage&              image,
                                    VkDeviceMemory&       imageMemory) {
  VkImageCreateInfo imageInfo = {};
  imageInfo.sType             = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo.imageType         = VK_IMAGE_TYPE_2D;
  imageInfo.extent.width      = width;
  imageInfo.extent.height     = height;
  imageInfo.extent.depth      = 1;
  imageInfo.mipLevels         = mipLevels;
  imageInfo.arrayLayers       = arrayLayers;
  imageInfo.format            = format;

  imageInfo.tiling = tiling;

  imageInfo.initialLayout = imageLayout;
  imageInfo.usage         = usage;

  imageInfo.samples = numSamples;
  imageInfo.flags   = imageCreateFlagBits;  // Optional

  if (vkCreateImage(g_rhiVk->m_device_, &imageInfo, nullptr, &image)
      != VK_SUCCESS) {
    GlobalLogger::Log(LogLevel::Error, "Failed to create 2D array image");
    return false;
  }

  VkMemoryRequirements memRequirements;
  vkGetImageMemoryRequirements(g_rhiVk->m_device_, image, &memRequirements);

  VkMemoryAllocateInfo allocInfo = {};
  allocInfo.sType                = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize       = memRequirements.size;
  allocInfo.memoryTypeIndex      = g_findMemoryType(
      g_rhiVk->m_physicalDevice_, memRequirements.memoryTypeBits, properties);
  if (vkAllocateMemory(g_rhiVk->m_device_, &allocInfo, nullptr, &imageMemory)
      != VK_SUCCESS) {
    GlobalLogger::Log(LogLevel::Error,
                      "Failed to allocate memory for 2D array image");
    return false;
  }

  vkBindImageMemory(g_rhiVk->m_device_, image, imageMemory, 0);

  return true;
}

std::shared_ptr<TextureVk> g_createTexture2DArray(
    uint32_t              width,
    uint32_t              height,
    uint32_t              arrayLayers,
    uint32_t              mipLevels,
    VkSampleCountFlagBits numSamples,
    VkFormat              format,
    VkImageTiling         tiling,
    VkImageUsageFlags     usage,
    VkMemoryPropertyFlags properties,
    VkImageCreateFlagBits imageCreateFlags,
    VkImageLayout         imageLayout) {
  auto TexturePtr = std::make_shared<TextureVk>();
  if (g_createTexture2DArrayLowLevel(width,
                                     height,
                                     arrayLayers,
                                     mipLevels,
                                     numSamples,
                                     format,
                                     tiling,
                                     usage,
                                     properties,
                                     imageLayout,
                                     imageCreateFlags,
                                     TexturePtr->m_image_,
                                     TexturePtr->m_deviceMemory_)) {
    TexturePtr->m_type_            = ETextureType::TEXTURE_2D_ARRAY;
    TexturePtr->m_extent_.width()  = width;
    TexturePtr->m_extent_.height() = height;
    TexturePtr->m_layerCount_      = arrayLayers;
    TexturePtr->m_mipLevels_       = mipLevels;
    TexturePtr->m_sampleCount_     = (EMSAASamples)numSamples;
    TexturePtr->m_format_          = g_getVulkanTextureFormat(format);
    TexturePtr->m_imageLayout_     = g_getVulkanImageLayout(imageLayout);

    const bool hasDepthAttachment = s_isDepthFormat(TexturePtr->m_format_);
    const VkImageAspectFlagBits ImageAspectFlagBit
        = hasDepthAttachment ? VK_IMAGE_ASPECT_DEPTH_BIT
                             : VK_IMAGE_ASPECT_COLOR_BIT;
    TexturePtr->m_imageView_ = g_createTextureView(
        TexturePtr->m_image_, format, ImageAspectFlagBit, mipLevels);
  } else {
    GlobalLogger::Log(LogLevel::Error, "Failed to create 2D array texture");
  }
  return TexturePtr;
}

bool g_create2DTextureLowLevel(uint32_t              width,
                               uint32_t              height,
                               uint32_t              mipLevels,
                               VkSampleCountFlagBits numSamples,
                               VkFormat              format,
                               VkImageTiling         tiling,
                               VkImageUsageFlags     usage,
                               VkMemoryPropertyFlags properties,
                               VkImageLayout         imageLayout,
                               VkImageCreateFlagBits imageCreateFlags,
                               VkImage&              image,
                               VkDeviceMemory&       imageMemory) {
  return g_createTexture2DArrayLowLevel(width,
                                        height,
                                        1,
                                        mipLevels,
                                        numSamples,
                                        format,
                                        tiling,
                                        usage,
                                        properties,
                                        imageLayout,
                                        imageCreateFlags,
                                        image,
                                        imageMemory);
}

std::shared_ptr<TextureVk> g_create2DTexture(
    uint32_t              width,
    uint32_t              height,
    uint32_t              mipLevels,
    VkSampleCountFlagBits numSamples,
    VkFormat              format,
    VkImageTiling         tiling,
    VkImageUsageFlags     usage,
    VkMemoryPropertyFlags properties,
    VkImageCreateFlagBits imageCreateFlags,
    VkImageLayout         imageLayout) {
  auto TexturePtr = std::make_shared<TextureVk>();
  if (g_create2DTextureLowLevel(width,
                                height,
                                mipLevels,
                                numSamples,
                                format,
                                tiling,
                                usage,
                                properties,
                                imageLayout,
                                imageCreateFlags,
                                TexturePtr->m_image_,
                                TexturePtr->m_deviceMemory_)) {
    TexturePtr->m_type_            = ETextureType::TEXTURE_2D;
    TexturePtr->m_extent_.width()  = width;
    TexturePtr->m_extent_.height() = height;
    TexturePtr->m_layerCount_      = 1;
    TexturePtr->m_mipLevels_       = mipLevels;
    TexturePtr->m_sampleCount_     = (EMSAASamples)numSamples;
    TexturePtr->m_format_          = g_getVulkanTextureFormat(format);
    TexturePtr->m_imageLayout_     = g_getVulkanImageLayout(imageLayout);

    const bool hasDepthAttachment = s_isDepthFormat(TexturePtr->m_format_);
    const VkImageAspectFlagBits ImageAspectFlagBit
        = hasDepthAttachment ? VK_IMAGE_ASPECT_DEPTH_BIT
                             : VK_IMAGE_ASPECT_COLOR_BIT;
    TexturePtr->m_imageView_ = g_createTextureView(
        TexturePtr->m_image_, format, ImageAspectFlagBit, mipLevels);
  }
  return TexturePtr;
}

bool g_createCubeTextureLowLevel(uint32_t              width,
                                 uint32_t              height,
                                 uint32_t              mipLevels,
                                 VkSampleCountFlagBits numSamples,
                                 VkFormat              format,
                                 VkImageTiling         tiling,
                                 VkImageUsageFlags     usage,
                                 VkMemoryPropertyFlags properties,
                                 VkImageCreateFlagBits imageCreateFlags,
                                 VkImageLayout         imageLayout,
                                 VkImage&              image,
                                 VkDeviceMemory&       imageMemory) {
  return g_createTexture2DArrayLowLevel(
      width,
      height,
      6,
      mipLevels,
      numSamples,
      format,
      tiling,
      usage,
      properties,
      imageLayout,
      (VkImageCreateFlagBits)(imageCreateFlags
                              | VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT),
      image,
      imageMemory);
}

std::shared_ptr<TextureVk> g_createCubeTexture(
    uint32_t              width,
    uint32_t              height,
    uint32_t              mipLevels,
    VkSampleCountFlagBits numSamples,
    VkFormat              format,
    VkImageTiling         tiling,
    VkImageUsageFlags     usage,
    VkMemoryPropertyFlags properties,
    VkImageCreateFlagBits imageCreateFlags,
    VkImageLayout         imageLayout) {
  auto TexturePtr = std::make_shared<TextureVk>();
  if (g_createCubeTextureLowLevel(width,
                                  height,
                                  mipLevels,
                                  numSamples,
                                  format,
                                  tiling,
                                  usage,
                                  properties,
                                  imageCreateFlags,
                                  imageLayout,
                                  TexturePtr->m_image_,
                                  TexturePtr->m_deviceMemory_)) {
    TexturePtr->m_type_            = ETextureType::TEXTURE_CUBE;
    TexturePtr->m_extent_.width()  = width;
    TexturePtr->m_extent_.height() = height;
    TexturePtr->m_layerCount_      = 6;
    TexturePtr->m_mipLevels_       = mipLevels;
    TexturePtr->m_sampleCount_     = (EMSAASamples)numSamples;
    TexturePtr->m_format_          = g_getVulkanTextureFormat(format);
    TexturePtr->m_imageLayout_     = g_getVulkanImageLayout(imageLayout);

    const bool hasDepthAttachment = s_isDepthFormat(TexturePtr->m_format_);
    const VkImageAspectFlagBits ImageAspectFlagBit
        = hasDepthAttachment ? VK_IMAGE_ASPECT_DEPTH_BIT
                             : VK_IMAGE_ASPECT_COLOR_BIT;
    TexturePtr->m_imageView_ = g_createTextureCubeView(
        TexturePtr->m_image_, format, ImageAspectFlagBit, mipLevels);
  } else {
    GlobalLogger::Log(LogLevel::Error, "Failed to create cube texture");
  }
  return TexturePtr;
}

uint32_t g_findMemoryType(VkPhysicalDevice      physicalDevice,
                          uint32_t              typeFilter,
                          VkMemoryPropertyFlags properties) {
  VkPhysicalDeviceMemoryProperties memProperties;
  vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

  for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i) {
    if ((typeFilter & (1 << i))
        && (memProperties.memoryTypes[i].propertyFlags & properties)
               == properties) {
      return i;
    }
  }

  GlobalLogger::Log(LogLevel::Error, "Failed to find suitable memory type");
  return 0;
}

size_t g_createBufferLowLevel(EVulkanBufferBits usage,
                              EVulkanMemoryBits properties,
                              uint64_t          size,
                              VkBuffer&         buffer,
                              VkDeviceMemory&   bufferMemory,
                              uint64_t&         allocateSize) {
  assert(size);
  VkBufferCreateInfo bufferInfo = {};
  bufferInfo.sType              = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size               = size;
  bufferInfo.usage              = g_getVulkanBufferBits(usage);
  bufferInfo.sharingMode        = VK_SHARING_MODE_EXCLUSIVE;

  if (!vkCreateBuffer(g_rhiVk->m_device_, &bufferInfo, nullptr, &buffer)
      == VK_SUCCESS) {
    GlobalLogger::Log(LogLevel::Error, "Failed to create buffer");
    return 0;
  }

  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(g_rhiVk->m_device_, buffer, &memRequirements);

  VkMemoryAllocateInfo allocInfo = {};
  allocInfo.sType                = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize       = memRequirements.size;
  allocInfo.memoryTypeIndex
      = g_findMemoryType(g_rhiVk->m_physicalDevice_,
                         memRequirements.memoryTypeBits,
                         g_getVulkanMemoryPropertyFlagBits(properties));

  if (!vkAllocateMemory(g_rhiVk->m_device_, &allocInfo, nullptr, &bufferMemory)
      == VK_SUCCESS) {
    GlobalLogger::Log(LogLevel::Error, "Failed to allocate buffer memory");
    return 0;
  }

  allocateSize = memRequirements.size;
  if (vkBindBufferMemory(g_rhiVk->m_device_, buffer, bufferMemory, 0)
      != VK_SUCCESS) {
    GlobalLogger::Log(LogLevel::Error, "Failed to bind buffer memory");
  }

  return memRequirements.size;
}

std::shared_ptr<BufferVk> g_createBuffer(EVulkanBufferBits usage,
                                         EVulkanMemoryBits properties,
                                         uint64_t          size,
                                         EResourceLayout   resourceLayout) {
  auto BufferPtr               = std::make_shared<BufferVk>();
  BufferPtr->m_realBufferSize_ = size;
#if USE_VK_MEMORY_POOL
  assert(g_rhi->getMemoryPool());
  const Memory& m_memory
      = g_rhi->getMemoryPool()->alloc(usage, properties, size);
  BufferPtr->initializeWithMemory(m_memory);
#else
  if (!g_createBufferLowLevel(g_rhiVk->m_device_,
                              g_rhiVk->m_physicalDevice_,
                              usage,
                              properties,
                              size,
                              BufferPtr->m_buffer,
                              BufferPtr->m_deviceMemory,
                              BufferPtr->AllocatedSize)) {
    GlobalLogger::Log(LogLevel::Error, "Failed to create buffer");
    return nullptr;
  }
#endif

  // TODO: consider whether it's needed to use transition layout for buffer for
  // now
  /*if (BufferPtr->Layout != resourceLayout) {
    g_rhi->transitionLayoutImmediate(BufferPtr.get(), resourceLayout);
  }*/

  return BufferPtr;
}

// size_t AllocateBuffer(VkBufferUsageFlags    usage,
//                       VkMemoryPropertyFlags properties,
//                       uint64_t              size,
//                       BufferVk&             buffer) {
// #if USE_VK_MEMORY_POOL
//// TODO: add memory pool logic
// #else
//   return g_createBuffer(g_rhiVk->m_device_,
//                       g_rhiVk->m_physicalDevice_,
//                       usage,
//                       properties,
//                       size,
//                       buffer.m_buffer,
//                       buffer.m_deviceMemory,
//                       buffer.AllocatedSize);
// #endif
// }

void g_copyBuffer(VkCommandBuffer commandBuffer,
                  VkBuffer        srcBuffer,
                  VkBuffer        dstBuffer,
                  VkDeviceSize    size,
                  VkDeviceSize    srcOffset,
                  VkDeviceSize    dstOffset) {
  VkBufferCopy copyRegion = {};
  copyRegion.srcOffset    = srcOffset;  // Optional
  copyRegion.dstOffset    = dstOffset;  // Optional
  copyRegion.size         = size;
  vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
}

void g_copyBuffer(VkCommandBuffer commandBuffer,
                  const BufferVk& srcBuffer,
                  const BufferVk& dstBuffer,
                  VkDeviceSize    size) {
  assert(srcBuffer.m_allocatedSize_ >= size
         && dstBuffer.m_allocatedSize_ >= size);
  g_copyBuffer(commandBuffer,
               srcBuffer.m_buffer_,
               dstBuffer.m_buffer_,
               size,
               srcBuffer.m_offset_,
               dstBuffer.m_offset_);
}

void g_copyBufferToTexture(VkCommandBuffer commandBuffer,
                           VkBuffer        buffer,
                           uint64_t        bufferOffset,
                           VkImage         image,
                           uint32_t        width,
                           uint32_t        height,
                           int32_t         miplevel,
                           int32_t         layerIndex) {
  VkBufferImageCopy region               = {};
  region.bufferOffset                    = bufferOffset;
  region.bufferRowLength                 = width;
  region.bufferImageHeight               = height;
  region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
  region.imageSubresource.mipLevel       = miplevel;
  region.imageSubresource.baseArrayLayer = layerIndex;
  region.imageSubresource.layerCount     = 1;
  region.imageOffset                     = {0, 0, 0};
  region.imageExtent                     = {width, height, 1};
  vkCmdCopyBufferToImage(commandBuffer,
                         buffer,
                         image,
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                         1,
                         &region);
}

void g_copyBuffer(VkBuffer     srcBuffer,
                  VkBuffer     dstBuffer,
                  VkDeviceSize size,
                  VkDeviceSize srcOffset,
                  VkDeviceSize dstOffset) {
  auto commandBuffer = std::static_pointer_cast<CommandBufferVk>(
      g_rhiVk->beginSingleTimeCommands());
  g_copyBuffer(commandBuffer->getRef(),
               srcBuffer,
               dstBuffer,
               size,
               srcOffset,
               dstOffset);
  g_rhiVk->endSingleTimeCommands(commandBuffer);
}

void g_copyBuffer(const BufferVk& srcBuffer,
                  const BufferVk& dstBuffer,
                  VkDeviceSize    size) {
  assert(srcBuffer.m_allocatedSize_ >= size
         && dstBuffer.m_allocatedSize_ >= size);
  auto commandBuffer = std::static_pointer_cast<CommandBufferVk>(
      g_rhiVk->beginSingleTimeCommands());
  g_copyBuffer(commandBuffer->getRef(),
               srcBuffer.m_buffer_,
               dstBuffer.m_buffer_,
               size,
               srcBuffer.m_offset_,
               dstBuffer.m_offset_);
  g_rhiVk->endSingleTimeCommands(commandBuffer);
}

void g_copyBufferToTexture(VkBuffer buffer,
                           uint64_t bufferOffset,
                           VkImage  image,
                           uint32_t width,
                           uint32_t height,
                           int32_t  miplevel,
                           int32_t  layerIndex) {
  auto commandBuffer = std::static_pointer_cast<CommandBufferVk>(
      g_rhiVk->beginSingleTimeCommands());
  g_copyBufferToTexture(commandBuffer->getRef(),
                        buffer,
                        bufferOffset,
                        image,
                        width,
                        height,
                        miplevel,
                        layerIndex);
  g_rhiVk->endSingleTimeCommands(commandBuffer);
}

}  // namespace game_engine
