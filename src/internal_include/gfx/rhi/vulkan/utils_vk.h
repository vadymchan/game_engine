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
    debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
                  VkDebugUtilsMessageTypeFlagsEXT             messageType,
                  const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                  void*                                       pUserData);

struct SwapChainSupportDetails {
  VkSurfaceCapabilitiesKHR        m_capabilities_;
  std::vector<VkSurfaceFormatKHR> m_formats_;
  std::vector<VkPresentModeKHR>   m_presentModes_;
};

SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device,
                                              VkSurfaceKHR     surface);

VkPresentModeKHR ChooseSwapPresentMode(
    const std::vector<VkPresentModeKHR>& availablePresentModes,
    bool                                 isVSyncEnabled);

struct QueueFamilyIndices {
  bool IsComplete() const {
    return m_graphicsFamily_.has_value() && m_computeFamily_.has_value()
        && m_presentFamily_.has_value();
  }

  std::optional<uint32_t> m_graphicsFamily_;
  std::optional<uint32_t> m_computeFamily_;
  std::optional<uint32_t> m_presentFamily_;
};

QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device,
                                     VkSurfaceKHR     surface);

VkImageView CreateImageView(VkDevice device, VkImage image, VkFormat format);

VkSemaphore CreateSemaphore(VkDevice device);

VkFence CreateFence(VkDevice device, bool signaled = false);

bool IsDepthFormat(VkFormat format);

bool IsDepthOnlyFormat(VkFormat format);

bool isTextureCube(VkImageViewType    imageViewType,
                   uint32_t           layerCount,
                   VkImageCreateFlags flags);

bool isTexture2DArray(VkImageViewType    imageViewType,
                      uint32_t           layerCount,
                      VkImageCreateFlags flags);

// =============== m_buffer Utils ===============

VkImageView CreateTextureView(VkImage            image,
                              VkFormat           format,
                              VkImageAspectFlags aspectMask,
                              uint32_t           mipLevels);

VkImageView CreateTexture2DArrayView(VkImage            image,
                                     uint32_t           layerCount,
                                     VkFormat           format,
                                     VkImageAspectFlags aspectMask,
                                     uint32_t           mipLevels);

VkImageView CreateTextureCubeView(VkImage            image,
                                  VkFormat           format,
                                  VkImageAspectFlags aspectMask,
                                  uint32_t           mipLevels);

VkImageView CreateTextureViewForSpecificMipMap(VkImage            image,
                                               VkFormat           format,
                                               VkImageAspectFlags aspectMask,
                                               uint32_t mipLevelIndex);

VkImageView CreateTexture2DArrayViewForSpecificMipMap(
    VkImage            image,
    uint32_t           layerCount,
    VkFormat           format,
    VkImageAspectFlags aspectMask,
    uint32_t           mipLevelIndex);

VkImageView CreateTextureCubeViewForSpecificMipMap(
    VkImage            image,
    VkFormat           format,
    VkImageAspectFlags aspectMask,
    uint32_t           mipLevelIndex);

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
                                   VkDeviceMemory&       imageMemory);

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
    VkImageLayout         imageLayout);
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
                              VkDeviceMemory&       imageMemory);

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
    VkImageLayout         imageLayout);

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
                                VkDeviceMemory&       imageMemory);

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
    VkImageLayout         imageLayout);

uint32_t FindMemoryType(VkPhysicalDevice      physicalDevice,
                        uint32_t              typeFilter,
                        VkMemoryPropertyFlags properties);

size_t CreateBuffer_LowLevel(EVulkanBufferBits usage,
                             EVulkanMemoryBits properties,
                             uint64_t          size,
                             VkBuffer&         OutBuffer,
                             VkDeviceMemory&   OutBufferMemory,
                             uint64_t&         OutAllocatedSize);

std::shared_ptr<BufferVk> CreateBuffer(EVulkanBufferBits usage,
                                       EVulkanMemoryBits properties,
                                       uint64_t          size,
                                       EResourceLayout   resourceLayout);

// size_t AllocateBuffer(VkBufferUsageFlags    usage,
//                       VkMemoryPropertyFlags properties,
//                       uint64_t              size,
//                       BufferVk&             OutBuffer);

void CopyBuffer(VkCommandBuffer commandBuffer,
                VkBuffer        srcBuffer,
                VkBuffer        dstBuffer,
                VkDeviceSize    size,
                VkDeviceSize    srcOffset,
                VkDeviceSize    dstOffset);

void CopyBuffer(VkCommandBuffer commandBuffer,
                const BufferVk& srcBuffer,
                const BufferVk& dstBuffer,
                VkDeviceSize    size);

void CopyBufferToTexture(VkCommandBuffer commandBuffer,
                         VkBuffer        buffer,
                         uint64_t        bufferOffset,
                         VkImage         image,
                         uint32_t        width,
                         uint32_t        height,
                         int32_t         miplevel   = 0,
                         int32_t         layerIndex = 0);

void CopyBuffer(VkBuffer     srcBuffer,
                VkBuffer     dstBuffer,
                VkDeviceSize size,
                VkDeviceSize srcOffset,
                VkDeviceSize dstOffset);

void CopyBuffer(const BufferVk& srcBuffer,
                const BufferVk& dstBuffer,
                VkDeviceSize    size);

void CopyBufferToTexture(VkBuffer buffer,
                         uint64_t bufferOffset,
                         VkImage  image,
                         uint32_t width,
                         uint32_t height,
                         int32_t  miplevel   = 0,
                         int32_t  layerIndex = 0);

}  // namespace game_engine

#endif  // GAME_ENGINE_UTILS_VK_H