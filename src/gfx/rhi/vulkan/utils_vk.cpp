
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
    debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
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

SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device,
                                              VkSurfaceKHR     surface) {
  SwapChainSupportDetails details;

  // Retrieve the basic surface capabilities.
  VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
      device, surface, &details.Capabilities);
  if (result != VK_SUCCESS) {
    GlobalLogger::Log(LogLevel::Error,
                      "Failed to get physical device surface capabilities");
  }

  // Retrieve the supported surface formats.
  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
  if (formatCount != 0) {
    details.Formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(
        device, surface, &formatCount, details.Formats.data());
  }

  // Retrieve the supported presentation modes.
  uint32_t presentModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(
      device, surface, &presentModeCount, nullptr);
  if (presentModeCount != 0) {
    details.PresentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        device, surface, &presentModeCount, details.PresentModes.data());
  }

  // Future expansions:
  // add additional checks or preferences here, such as prioritizing
  // certain formats or present modes based on performance or specific
  // requirements of engine.

  return details;
}

VkPresentModeKHR ChooseSwapPresentMode(
    const std::vector<VkPresentModeKHR>& availablePresentModes,
    bool                                 isVSyncEnabled) {
  if (isVSyncEnabled) {
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

QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device,
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
      indices.GraphicsFamily = queueFamilyIndex;
    }

    // Check for Presentation Support
    VkBool32 supportsPresent = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(
        device, queueFamilyIndex, surface, &supportsPresent);
    if (supportsPresent) {
      indices.PresentFamily = queueFamilyIndex;
    }

    // Check for Compute Queue Support
    if (queueFamilyProperties.queueFlags & VK_QUEUE_COMPUTE_BIT) {
      indices.ComputeFamily = queueFamilyIndex;
    }

    if (indices.IsComplete()) {
      break;
    }

    ++queueFamilyIndex;
  }

  return indices;
}

VkSemaphore CreateSemaphore() {
  VkSemaphoreCreateInfo semaphoreInfo{};
  semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkSemaphore semaphore;
  if (vkCreateSemaphore(
          g_rhi_vk->m_device_, &semaphoreInfo, nullptr, &semaphore)
      != VK_SUCCESS) {
    GlobalLogger::Log(LogLevel::Error, "Failed to create semaphore");
  }

  return semaphore;
}

VkFence CreateFence(bool signaled) {
  VkFenceCreateInfo fenceInfo{};
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceInfo.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT
                             : 0;  // Optionally create it in a signaled state

  VkFence fence;
  if (vkCreateFence(g_rhi_vk->m_device_, &fenceInfo, nullptr, &fence)
      != VK_SUCCESS) {
    GlobalLogger::Log(LogLevel::Error, "Failed to create fence");
  }

  return fence;
}

// =============== Buffer Utils ===============

// --------------- image (texture) ------------

bool IsDepthFormat(VkFormat format) {
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

bool IsDepthOnlyFormat(VkFormat format) {
  switch (format) {
    case VK_FORMAT_D16_UNORM:
    case VK_FORMAT_X8_D24_UNORM_PACK32:
    case VK_FORMAT_D32_SFLOAT:
      return true;
  }

  return false;
}

bool isTextureCube(VkImageViewType    imageViewType,
                   uint32_t           layerCount,
                   VkImageCreateFlags flags) {
  return imageViewType == VK_IMAGE_VIEW_TYPE_CUBE
      || imageViewType == VK_IMAGE_VIEW_TYPE_CUBE_ARRAY
             && (flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT);
}

bool isTexture2DArray(VkImageViewType    imageViewType,
                      uint32_t           layerCount,
                      VkImageCreateFlags flags) {
  return (imageViewType == VK_IMAGE_VIEW_TYPE_2D_ARRAY) && layerCount > 1
      && !(flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT);
}

VkImageView CreateTextureView(VkImage            image,
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
  if (vkCreateImageView(g_rhi_vk->m_device_, &viewInfo, nullptr, &imageView)
      != VK_SUCCESS) {
    GlobalLogger::Log(LogLevel::Error, "Failed to create image view");
  }

  return imageView;
}

VkImageView CreateTexture2DArrayView(VkImage            image,
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
  if (vkCreateImageView(g_rhi_vk->m_device_, &viewInfo, nullptr, &imageView)
      != VK_SUCCESS) {
    GlobalLogger::Log(LogLevel::Error, "Failed to create 2D array image view");
  }

  return imageView;
}

VkImageView CreateTextureCubeView(VkImage            image,
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
  if (vkCreateImageView(g_rhi_vk->m_device_, &viewInfo, nullptr, &imageView)
      != VK_SUCCESS) {
    GlobalLogger::Log(LogLevel::Error, "Failed to create cube image view");
  }

  return imageView;
}

VkImageView CreateTextureViewForSpecificMipMap(VkImage            image,
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
  if (vkCreateImageView(g_rhi_vk->m_device_, &viewInfo, nullptr, &imageView)
      != VK_SUCCESS) {
    GlobalLogger::Log(LogLevel::Error,
                      "Failed to create image view for specific mip level");
  }

  return imageView;
}

VkImageView CreateTexture2DArrayViewForSpecificMipMap(
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
  if (vkCreateImageView(g_rhi_vk->m_device_, &viewInfo, nullptr, &imageView)
      != VK_SUCCESS) {
    GlobalLogger::Log(
        LogLevel::Error,
        "Failed to create 2D array image view for specific mip level");
  }

  return imageView;
}

VkImageView CreateTextureCubeViewForSpecificMipMap(
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
  if (vkCreateImageView(g_rhi_vk->m_device_, &viewInfo, nullptr, &imageView)
      != VK_SUCCESS) {
    GlobalLogger::Log(
        LogLevel::Error,
        "Failed to create cube image view for specific mip level");
  }

  return imageView;
}

bool CreateTexture2DArray_LowLevel(uint32_t              width,
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

  if (vkCreateImage(g_rhi_vk->m_device_, &imageInfo, nullptr, &image)
      != VK_SUCCESS) {
    GlobalLogger::Log(LogLevel::Error, "Failed to create 2D array image");
    return false;
  }

  VkMemoryRequirements memRequirements;
  vkGetImageMemoryRequirements(g_rhi_vk->m_device_, image, &memRequirements);

  VkMemoryAllocateInfo allocInfo = {};
  allocInfo.sType                = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize       = memRequirements.size;
  allocInfo.memoryTypeIndex      = FindMemoryType(
      g_rhi_vk->m_physicalDevice_, memRequirements.memoryTypeBits, properties);
  if (vkAllocateMemory(g_rhi_vk->m_device_, &allocInfo, nullptr, &imageMemory)
      != VK_SUCCESS) {
    GlobalLogger::Log(LogLevel::Error,
                      "Failed to allocate memory for 2D array image");
    return false;
  }

  vkBindImageMemory(g_rhi_vk->m_device_, image, imageMemory, 0);

  return true;
}

std::shared_ptr<TextureVk> CreateTexture2DArray(
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
  if (CreateTexture2DArray_LowLevel(width,
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
                                    TexturePtr->image,
                                    TexturePtr->deviceMemory)) {
    TexturePtr->type            = ETextureType::TEXTURE_2D_ARRAY;
    TexturePtr->extent.width()  = width;
    TexturePtr->extent.height() = height;
    TexturePtr->layerCount      = arrayLayers;
    TexturePtr->mipLevels       = mipLevels;
    TexturePtr->sampleCount     = (EMSAASamples)numSamples;
    TexturePtr->format          = GetVulkanTextureFormat(format);
    TexturePtr->imageLayout     = GetVulkanImageLayout(imageLayout);

    const bool hasDepthAttachment = IsDepthFormat(TexturePtr->format);
    const VkImageAspectFlagBits ImageAspectFlagBit
        = hasDepthAttachment ? VK_IMAGE_ASPECT_DEPTH_BIT
                             : VK_IMAGE_ASPECT_COLOR_BIT;
    TexturePtr->imageView = CreateTextureView(
        TexturePtr->image, format, ImageAspectFlagBit, mipLevels);
  } else {
    GlobalLogger::Log(LogLevel::Error, "Failed to create 2D array texture");
  }
  return TexturePtr;
}

bool Create2DTexture_LowLevel(uint32_t              width,
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
  return CreateTexture2DArray_LowLevel(width,
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

std::shared_ptr<TextureVk> Create2DTexture(
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
  if (Create2DTexture_LowLevel(width,
                               height,
                               mipLevels,
                               numSamples,
                               format,
                               tiling,
                               usage,
                               properties,
                               imageLayout,
                               imageCreateFlags,
                               TexturePtr->image,
                               TexturePtr->deviceMemory)) {
    TexturePtr->type            = ETextureType::TEXTURE_2D;
    TexturePtr->extent.width()  = width;
    TexturePtr->extent.height() = height;
    TexturePtr->layerCount      = 1;
    TexturePtr->mipLevels       = mipLevels;
    TexturePtr->sampleCount     = (EMSAASamples)numSamples;
    TexturePtr->format          = GetVulkanTextureFormat(format);
    TexturePtr->imageLayout     = GetVulkanImageLayout(imageLayout);

    const bool hasDepthAttachment = IsDepthFormat(TexturePtr->format);
    const VkImageAspectFlagBits ImageAspectFlagBit
        = hasDepthAttachment ? VK_IMAGE_ASPECT_DEPTH_BIT
                             : VK_IMAGE_ASPECT_COLOR_BIT;
    TexturePtr->imageView = CreateTextureView(
        TexturePtr->image, format, ImageAspectFlagBit, mipLevels);
  }
  return TexturePtr;
}

bool CreateCubeTexture_LowLevel(uint32_t              width,
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
  return CreateTexture2DArray_LowLevel(
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

std::shared_ptr<TextureVk> CreateCubeTexture(
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
  if (CreateCubeTexture_LowLevel(width,
                                 height,
                                 mipLevels,
                                 numSamples,
                                 format,
                                 tiling,
                                 usage,
                                 properties,
                                 imageCreateFlags,
                                 imageLayout,
                                 TexturePtr->image,
                                 TexturePtr->deviceMemory)) {
    TexturePtr->type            = ETextureType::TEXTURE_CUBE;
    TexturePtr->extent.width()  = width;
    TexturePtr->extent.height() = height;
    TexturePtr->layerCount      = 6;
    TexturePtr->mipLevels       = mipLevels;
    TexturePtr->sampleCount     = (EMSAASamples)numSamples;
    TexturePtr->format          = GetVulkanTextureFormat(format);
    TexturePtr->imageLayout     = GetVulkanImageLayout(imageLayout);

    const bool hasDepthAttachment = IsDepthFormat(TexturePtr->format);
    const VkImageAspectFlagBits ImageAspectFlagBit
        = hasDepthAttachment ? VK_IMAGE_ASPECT_DEPTH_BIT
                             : VK_IMAGE_ASPECT_COLOR_BIT;
    TexturePtr->imageView = CreateTextureCubeView(
        TexturePtr->image, format, ImageAspectFlagBit, mipLevels);
  } else {
    GlobalLogger::Log(LogLevel::Error, "Failed to create cube texture");
  }
  return TexturePtr;
}

uint32_t FindMemoryType(VkPhysicalDevice      physicalDevice,
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

size_t CreateBuffer_LowLevel(EVulkanBufferBits InUsage,
                             EVulkanMemoryBits InProperties,
                             uint64_t          InSize,
                             VkBuffer&         OutBuffer,
                             VkDeviceMemory&   OutBufferMemory,
                             uint64_t&         OutAllocatedSize) {
  assert(InSize);
  VkBufferCreateInfo bufferInfo = {};
  bufferInfo.sType              = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size               = InSize;
  bufferInfo.usage              = GetVulkanBufferBits(InUsage);
  bufferInfo.sharingMode        = VK_SHARING_MODE_EXCLUSIVE;

  if (!vkCreateBuffer(g_rhi_vk->m_device_, &bufferInfo, nullptr, &OutBuffer)
      == VK_SUCCESS) {
    GlobalLogger::Log(LogLevel::Error, "Failed to create buffer");
    return 0;
  }

  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(
      g_rhi_vk->m_device_, OutBuffer, &memRequirements);

  VkMemoryAllocateInfo allocInfo = {};
  allocInfo.sType                = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize       = memRequirements.size;
  allocInfo.memoryTypeIndex
      = FindMemoryType(g_rhi_vk->m_physicalDevice_,
                       memRequirements.memoryTypeBits,
                       GetVulkanMemoryPropertyFlagBits(InProperties));

  if (!vkAllocateMemory(
          g_rhi_vk->m_device_, &allocInfo, nullptr, &OutBufferMemory)
      == VK_SUCCESS) {
    GlobalLogger::Log(LogLevel::Error, "Failed to allocate buffer memory");
    return 0;
  }

  OutAllocatedSize = memRequirements.size;
  if (vkBindBufferMemory(g_rhi_vk->m_device_, OutBuffer, OutBufferMemory, 0)
      != VK_SUCCESS) {
    GlobalLogger::Log(LogLevel::Error, "Failed to bind buffer memory");
  }

  return memRequirements.size;
}

std::shared_ptr<BufferVk> CreateBuffer(EVulkanBufferBits InUsage,
                                       EVulkanMemoryBits InProperties,
                                       uint64_t          InSize,
                                       EResourceLayout   InResourceLayout) {
  auto BufferPtr            = std::make_shared<BufferVk>();
  BufferPtr->RealBufferSize = InSize;
#if USE_VK_MEMORY_POOL
  assert(g_rhi->GetMemoryPool());
  const Memory& memory
      = g_rhi->GetMemoryPool()->Alloc(InUsage, InProperties, InSize);
  BufferPtr->InitializeWithMemory(memory);
#else
  if (!CreateBuffer_LowLevel(g_rhi_vk->m_device_,
                             g_rhi_vk->m_physicalDevice_,
                             InUsage,
                             InProperties,
                             InSize,
                             BufferPtr->m_buffer,
                             BufferPtr->m_deviceMemory,
                             BufferPtr->AllocatedSize)) {
    GlobalLogger::Log(LogLevel::Error, "Failed to create buffer");
    return nullptr;
  }
#endif

  // TODO: consider whether it's needed to use transition layout for buffer for
  // now
  /*if (BufferPtr->Layout != InResourceLayout) {
    g_rhi->TransitionLayoutImmediate(BufferPtr.get(), InResourceLayout);
  }*/

  return BufferPtr;
}

// size_t AllocateBuffer(VkBufferUsageFlags    InUsage,
//                       VkMemoryPropertyFlags InProperties,
//                       uint64_t              InSize,
//                       BufferVk&             OutBuffer) {
// #if USE_VK_MEMORY_POOL
//// TODO: add memory pool logic
// #else
//   return CreateBuffer(g_rhi_vk->m_device_,
//                       g_rhi_vk->m_physicalDevice_,
//                       InUsage,
//                       InProperties,
//                       InSize,
//                       OutBuffer.m_buffer,
//                       OutBuffer.m_deviceMemory,
//                       OutBuffer.AllocatedSize);
// #endif
// }

void CopyBuffer(VkCommandBuffer commandBuffer,
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

void CopyBuffer(VkCommandBuffer commandBuffer,
                const BufferVk& srcBuffer,
                const BufferVk& dstBuffer,
                VkDeviceSize    size) {
  assert(srcBuffer.AllocatedSize >= size && dstBuffer.AllocatedSize >= size);
  CopyBuffer(commandBuffer,
             srcBuffer.m_buffer,
             dstBuffer.m_buffer,
             size,
             srcBuffer.Offset,
             dstBuffer.Offset);
}

void CopyBufferToTexture(VkCommandBuffer commandBuffer,
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

void CopyBuffer(VkBuffer     srcBuffer,
                VkBuffer     dstBuffer,
                VkDeviceSize size,
                VkDeviceSize srcOffset,
                VkDeviceSize dstOffset) {
  auto* commandBuffer = g_rhi_vk->BeginSingleTimeCommands();
  CopyBuffer(commandBuffer->GetRef(),
             srcBuffer,
             dstBuffer,
             size,
             srcOffset,
             dstOffset);
  g_rhi_vk->EndSingleTimeCommands(commandBuffer);
}

void CopyBuffer(const BufferVk& srcBuffer,
                const BufferVk& dstBuffer,
                VkDeviceSize    size) {
  assert(srcBuffer.AllocatedSize >= size && dstBuffer.AllocatedSize >= size);
  auto* commandBuffer = g_rhi_vk->BeginSingleTimeCommands();
  CopyBuffer(commandBuffer->GetRef(),
             srcBuffer.m_buffer,
             dstBuffer.m_buffer,
             size,
             srcBuffer.Offset,
             dstBuffer.Offset);
  g_rhi_vk->EndSingleTimeCommands(commandBuffer);
}

void CopyBufferToTexture(VkBuffer buffer,
                         uint64_t bufferOffset,
                         VkImage  image,
                         uint32_t width,
                         uint32_t height,
                         int32_t  miplevel,
                         int32_t  layerIndex) {
  auto* commandBuffer = g_rhi_vk->BeginSingleTimeCommands();
  CopyBufferToTexture(commandBuffer->GetRef(),
                      buffer,
                      bufferOffset,
                      image,
                      width,
                      height,
                      miplevel,
                      layerIndex);
  g_rhi_vk->EndSingleTimeCommands(commandBuffer);
}

}  // namespace game_engine