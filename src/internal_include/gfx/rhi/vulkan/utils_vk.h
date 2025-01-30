#ifndef GAME_ENGINE_UTILS_VK_H
#define GAME_ENGINE_UTILS_VK_H

#include "gfx/rhi/vulkan/rhi_type_vk.h"

#include <vulkan/vulkan.h>

#include <cassert>
#include <memory>
#include <optional>
#include <vector>

namespace game_engine {

// TODO: make sure this will work as intended
class BufferVk;
class TextureVk;

// callback function for Vulkan debug messages

VKAPI_ATTR VkBool32 VKAPI_CALL
    g_debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
                    VkDebugUtilsMessageTypeFlagsEXT             messageType,
                    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                    void*                                       pUserData);

struct SwapChainSupportDetails {
  // ======= BEGIN: public misc fields ========================================

  VkSurfaceCapabilitiesKHR        m_capabilities_;
  std::vector<VkSurfaceFormatKHR> m_formats_;
  std::vector<VkPresentModeKHR>   m_presentModes_;

  // ======= END: public misc fields   ========================================
};

SwapChainSupportDetails g_querySwapChainSupport(VkPhysicalDevice device,
                                                VkSurfaceKHR     surface);

VkPresentModeKHR g_chooseSwapPresentMode(
    const std::vector<VkPresentModeKHR>& availablePresentModes,
    bool                                 m_isVSyncEnabled_);

struct QueueFamilyIndices {
  // ======= BEGIN: public misc methods =======================================

  bool isComplete() const {
    return m_graphicsFamily_.has_value() && m_computeFamily_.has_value()
        && m_presentFamily_.has_value();
  }

  // ======= END: public misc methods   =======================================

  // ======= BEGIN: public misc fields ========================================

  std::optional<uint32_t> m_graphicsFamily_;
  std::optional<uint32_t> m_computeFamily_;
  std::optional<uint32_t> m_presentFamily_;

  // ======= END: public misc fields   ========================================
};

QueueFamilyIndices g_findQueueFamilies(VkPhysicalDevice device,
                                       VkSurfaceKHR     surface);

// TODO: not used
VkImageView g_createImageView(VkDevice device, VkImage image, VkFormat format);

VkSemaphore g_createSemaphore(VkDevice device);

VkFence g_createFence(VkDevice device, bool signaled = false);

bool g_isDepthFormat(VkFormat format);

bool g_isDepthOnlyFormat(VkFormat format);

bool g_isTextureCube(VkImageViewType    imageViewType,
                     uint32_t           layerCount,
                     VkImageCreateFlags flags);

bool g_isTexture2DArray(VkImageViewType    imageViewType,
                        uint32_t           layerCount,
                        VkImageCreateFlags flags);

// =============== m_buffer Utils ===============

VkImageView g_createTextureView(VkImage            image,
                                VkFormat           format,
                                VkImageAspectFlags aspectMask,
                                uint32_t           mipLevels);

VkImageView g_createTexture2DArrayView(VkImage            image,
                                       uint32_t           layerCount,
                                       VkFormat           format,
                                       VkImageAspectFlags aspectMask,
                                       uint32_t           mipLevels);

VkImageView g_createTextureCubeView(VkImage            image,
                                    VkFormat           format,
                                    VkImageAspectFlags aspectMask,
                                    uint32_t           mipLevels);

VkImageView g_createTextureViewForSpecificMipMap(VkImage            image,
                                                 VkFormat           format,
                                                 VkImageAspectFlags aspectMask,
                                                 uint32_t mipLevelIndex);

VkImageView g_createTexture2DArrayViewForSpecificMipMap(
    VkImage            image,
    uint32_t           layerCount,
    VkFormat           format,
    VkImageAspectFlags aspectMask,
    uint32_t           mipLevelIndex);

VkImageView g_createTextureCubeViewForSpecificMipMap(
    VkImage            image,
    VkFormat           format,
    VkImageAspectFlags aspectMask,
    uint32_t           mipLevelIndex);

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
                                    VkDeviceMemory&       imageMemory);

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
    VkImageLayout         imageLayout);

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
                               VkDeviceMemory&       imageMemory);

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
    VkImageLayout         imageLayout);

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
                                 VkDeviceMemory&       imageMemory);

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
    VkImageLayout         imageLayout);

uint32_t g_findMemoryType(VkPhysicalDevice      physicalDevice,
                          uint32_t              typeFilter,
                          VkMemoryPropertyFlags properties);

size_t g_createBufferLowLevel(EVulkanBufferBits usage,
                              EVulkanMemoryBits properties,
                              uint64_t          size,
                              VkBuffer&         buffer,
                              VkDeviceMemory&   bufferMemory,
                              uint64_t&         allocateSize);

std::shared_ptr<BufferVk> g_createBuffer(EVulkanBufferBits usage,
                                         EVulkanMemoryBits properties,
                                         uint64_t          size,
                                         EResourceLayout   resourceLayout);

// size_t AllocateBuffer(VkBufferUsageFlags    usage,
//                       VkMemoryPropertyFlags properties,
//                       uint64_t              size,
//                       BufferVk&             buffer);

void g_copyBuffer(VkCommandBuffer commandBuffer,
                  VkBuffer        srcBuffer,
                  VkBuffer        dstBuffer,
                  VkDeviceSize    size,
                  VkDeviceSize    srcOffset,
                  VkDeviceSize    dstOffset);

void g_copyBuffer(VkCommandBuffer commandBuffer,
                  const BufferVk& srcBuffer,
                  const BufferVk& dstBuffer,
                  VkDeviceSize    size);

void g_copyBufferToTexture(VkCommandBuffer commandBuffer,
                           VkBuffer        buffer,
                           uint64_t        bufferOffset,
                           VkImage         image,
                           uint32_t        width,
                           uint32_t        height,
                           int32_t         miplevel   = 0,
                           int32_t         layerIndex = 0);

void g_copyBuffer(VkBuffer     srcBuffer,
                  VkBuffer     dstBuffer,
                  VkDeviceSize size,
                  VkDeviceSize srcOffset,
                  VkDeviceSize dstOffset);

void g_copyBuffer(const BufferVk& srcBuffer,
                  const BufferVk& dstBuffer,
                  VkDeviceSize    size);

void g_copyBufferToTexture(VkBuffer buffer,
                           uint64_t bufferOffset,
                           VkImage  image,
                           uint32_t width,
                           uint32_t height,
                           int32_t  miplevel   = 0,
                           int32_t  layerIndex = 0);

void g_copyImage(VkCommandBuffer commandBuffer,
                 VkImage         srcImage,
                 VkImageLayout   srcLayout,
                 VkImage         dstImage,
                 VkImageLayout   dstLayout,
                 uint32_t        srcWidth,
                 uint32_t        srcHeight,
                 uint32_t        dstWidth,
                 uint32_t        dstHeight,
                 uint32_t        srcLayerCount = 1,
                 uint32_t        dstLayerCount = 1);

void g_copyTexture(VkCommandBuffer                   commandBuffer,
                   const std::shared_ptr<TextureVk>& srcTexture,
                   const std::shared_ptr<TextureVk>& dstTexture);

}  // namespace game_engine

#endif  // GAME_ENGINE_UTILS_VK_H
