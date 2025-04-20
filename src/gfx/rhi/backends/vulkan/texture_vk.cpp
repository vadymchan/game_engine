#include "gfx/rhi/backends/vulkan/texture_vk.h"

#include "gfx/rhi/backends/vulkan/command_buffer_vk.h"
#include "gfx/rhi/backends/vulkan/device_vk.h"
#include "gfx/rhi/backends/vulkan/rhi_enums_vk.h"
#include "gfx/rhi/backends/vulkan/synchronization_vk.h"
#include "utils/logger/global_logger.h"

namespace game_engine {
namespace gfx {
namespace rhi {

TextureVk::TextureVk(const TextureDesc& desc, DeviceVk* device)
    : Texture(desc)
    , m_device_(device) {
  m_vkFormat_ = g_getTextureFormatVk(desc.format);

  m_currentLayout_ = desc.initialLayout;

  // Only need to call createImage_() and createImageView_() now - VMA handles memory allocation
  if (!createImage_() || !createImageView_()) {
    GlobalLogger::Log(LogLevel::Error, "Failed to create Vulkan texture");
  }
}

TextureVk::TextureVk(DeviceVk* device, const TextureDesc& desc, VkImage existingImage, VkImageView existingImageView)
    : Texture(desc)
    , m_device_(device)
    , m_image_(existingImage)
    , m_imageView_(existingImageView)
    , m_ownsImage_(false) {
  m_vkFormat_ = g_getTextureFormatVk(desc.format);

  m_currentLayout_ = desc.initialLayout;
}

TextureVk::~TextureVk() {
  if (m_device_) {
    VkDevice device = m_device_->getDevice();

    if (m_imageView_ != VK_NULL_HANDLE) {
      vkDestroyImageView(device, m_imageView_, nullptr);
      m_imageView_ = VK_NULL_HANDLE;
    }

    if (m_ownsImage_) {
      if (m_image_ != VK_NULL_HANDLE) {
        // Use VMA to destroy the image and its allocation
        vmaDestroyImage(m_device_->getAllocator(), m_image_, m_allocation_);
        m_image_      = VK_NULL_HANDLE;
        m_allocation_ = VK_NULL_HANDLE;
      }
    }
  }
}

bool TextureVk::createImage_() {
  VkImageCreateInfo imageInfo = {};
  imageInfo.sType             = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;

  switch (m_desc_.type) {
    case TextureType::Texture1D:
      imageInfo.imageType = VK_IMAGE_TYPE_1D;
      break;
    case TextureType::Texture2D:
    case TextureType::Texture2DArray:
    case TextureType::TextureCube:
      imageInfo.imageType = VK_IMAGE_TYPE_2D;
      break;
    case TextureType::Texture3D:
    case TextureType::Texture3DArray:
      imageInfo.imageType = VK_IMAGE_TYPE_3D;
      break;
    default:
      GlobalLogger::Log(LogLevel::Error, "Unsupported texture type");
      return false;
  }

  imageInfo.format        = m_vkFormat_;
  imageInfo.extent.width  = m_desc_.width;
  imageInfo.extent.height = m_desc_.height;
  imageInfo.extent.depth  = m_desc_.depth;
  imageInfo.mipLevels     = m_desc_.mipLevels;
  imageInfo.arrayLayers   = m_desc_.arraySize;

  if (m_desc_.type == TextureType::TextureCube) {
    imageInfo.arrayLayers  = 6;
    imageInfo.flags       |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
  }

  imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  if (m_desc_.sampleCount == MSAASamples::Count2) {
    imageInfo.samples = VK_SAMPLE_COUNT_2_BIT;
  } else if (m_desc_.sampleCount == MSAASamples::Count4) {
    imageInfo.samples = VK_SAMPLE_COUNT_4_BIT;
  } else if (m_desc_.sampleCount == MSAASamples::Count8) {
    imageInfo.samples = VK_SAMPLE_COUNT_8_BIT;
  } else if (m_desc_.sampleCount == MSAASamples::Count16) {
    imageInfo.samples = VK_SAMPLE_COUNT_16_BIT;
  }

  imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;

  imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT;  // Always allow sampling in shaders

  if (hasRtvUsage()) {
    imageInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  }

  if (hasDsvUsage()) {
    imageInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
  }

  if (hasUavUsage()) {
    imageInfo.usage |= VK_IMAGE_USAGE_STORAGE_BIT;
  }

  // Always allow texture to be the target of transfer operations for uploads
  imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

  // For mip generation and transfers
  if (m_desc_.mipLevels > 1 || ((m_desc_.createFlags & TextureCreateFlag::TransferSrc) != TextureCreateFlag::None)) {
    imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
  }

  // Sharing and initial layout
  imageInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
  imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

  // VMA allocation info
  VmaAllocationCreateInfo allocInfo = {};
  allocInfo.usage                   = VMA_MEMORY_USAGE_GPU_ONLY;  // Textures typically use GPU-only memory

  // Create the image with VMA
  VkResult result = vmaCreateImage(
      m_device_->getAllocator(), &imageInfo, &allocInfo, &m_image_, &m_allocation_, &m_allocationInfo_);

  if (result != VK_SUCCESS) {
    GlobalLogger::Log(LogLevel::Error, "Failed to create image with VMA");
    return false;
  }

  return true;
}

bool TextureVk::createImageView_() {
  VkImageViewCreateInfo viewInfo = {};
  viewInfo.sType                 = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.image                 = m_image_;

  // Set view type based on texture type
  switch (m_desc_.type) {
    case TextureType::Texture1D:
      viewInfo.viewType = VK_IMAGE_VIEW_TYPE_1D;
      break;
    case TextureType::Texture2D:
      viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
      break;
    case TextureType::Texture2DArray:
      viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
      break;
    case TextureType::Texture3D:
      viewInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
      break;
    case TextureType::Texture3DArray:
      viewInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;  // No direct 3D array in Vulkan
      break;
    case TextureType::TextureCube:
      viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
      break;
    default:
      GlobalLogger::Log(LogLevel::Error, "Unsupported texture view type");
      return false;
  }

  // Set format
  viewInfo.format = m_vkFormat_;

  // Set swizzle (identity mapping)
  viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
  viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
  viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
  viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

  // Set subresource range
  viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

  // If this is a depth/stencil format, set the appropriate aspect mask
  if (g_isDepthFormat(m_desc_.format)) {
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

    // Add stencil aspect if the format has stencil component
    if (!g_isDepthOnlyFormat(m_desc_.format)) {
      viewInfo.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }
  }

  viewInfo.subresourceRange.baseMipLevel   = 0;
  viewInfo.subresourceRange.levelCount     = m_desc_.mipLevels;
  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.subresourceRange.layerCount     = m_desc_.arraySize;

  // Create the image view
  if (vkCreateImageView(m_device_->getDevice(), &viewInfo, nullptr, &m_imageView_) != VK_SUCCESS) {
    GlobalLogger::Log(LogLevel::Error, "Failed to create Vulkan image view");
    return false;
  }

  return true;
}

void TextureVk::updateCurrentLayout_(ResourceLayout layout) {
  m_currentLayout_ = layout;
}

void TextureVk::update(const void* data, size_t dataSize, uint32_t mipLevel, uint32_t arrayLayer) {
  if (!data || !dataSize) {
    GlobalLogger::Log(LogLevel::Error, "Invalid data or size for texture update");
    return;
  }

  // Create a staging buffer using VMA
  VmaAllocation stagingAllocation = VK_NULL_HANDLE;
  VkBuffer      stagingBuffer     = m_device_->createStagingBuffer(data, dataSize, stagingAllocation);

  if (stagingBuffer == VK_NULL_HANDLE) {
    GlobalLogger::Log(LogLevel::Error, "Failed to create staging buffer for texture update");
    return;
  }

  // Create a command buffer for the transfer
  CommandBufferDesc cmdBufferDesc;
  cmdBufferDesc.primary = true;

  auto commandBuffer = m_device_->createCommandBuffer(cmdBufferDesc);
  if (!commandBuffer) {
    // Clean up staging resources
    vmaDestroyBuffer(m_device_->getAllocator(), stagingBuffer, stagingAllocation);
    GlobalLogger::Log(LogLevel::Error, "Failed to create command buffer for texture update");
    return;
  }

  CommandBufferVk* cmdBufferVk = static_cast<CommandBufferVk*>(commandBuffer.get());

  cmdBufferVk->reset();

  // Begin command buffer
  cmdBufferVk->begin();

  // Transition layout to transfer destination
  ResourceBarrierDesc barrier = {};
  barrier.texture             = this;
  barrier.oldLayout           = m_currentLayout_;
  barrier.newLayout           = ResourceLayout::TransferDst;
  cmdBufferVk->resourceBarrier(barrier);

  // Calculate subresource dimensions for this mip level
  uint32_t mipWidth = m_desc_.width >> mipLevel;
  mipWidth          = mipWidth > 0 ? mipWidth : 1;

  uint32_t mipHeight = m_desc_.height >> mipLevel;
  mipHeight          = mipHeight > 0 ? mipHeight : 1;

  uint32_t mipDepth = m_desc_.depth >> mipLevel;
  mipDepth          = mipDepth > 0 ? mipDepth : 1;

  // Set up buffer to image copy
  VkBufferImageCopy region = {};
  region.bufferOffset      = 0;
  region.bufferRowLength   = 0;  // Tightly packed
  region.bufferImageHeight = 0;  // Tightly packed

  region.imageSubresource.aspectMask
      = g_isDepthFormat(m_desc_.format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
  region.imageSubresource.mipLevel       = mipLevel;
  region.imageSubresource.baseArrayLayer = arrayLayer;
  region.imageSubresource.layerCount     = 1;

  region.imageOffset = {0, 0, 0};
  region.imageExtent = {mipWidth, mipHeight, mipDepth};

  // Copy buffer to image
  vkCmdCopyBufferToImage(
      cmdBufferVk->getCommandBuffer(), stagingBuffer, m_image_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

  // Transition layout back to original
  barrier.oldLayout = ResourceLayout::TransferDst;
  barrier.newLayout = m_currentLayout_;
  cmdBufferVk->resourceBarrier(barrier);

  // End and submit command buffer
  cmdBufferVk->end();

  // Create a fence to wait for the upload to complete
  FenceDesc fenceDesc;
  fenceDesc.signaled = false;
  auto fence         = m_device_->createFence(fenceDesc);

  // Submit the command buffer
  m_device_->submitCommandBuffer(commandBuffer.get(), fence.get());

  // Wait for the upload to complete
  fence->wait();

  // Clean up staging resources using VMA
  vmaDestroyBuffer(m_device_->getAllocator(), stagingBuffer, stagingAllocation);
}
}  // namespace rhi
}  // namespace gfx
}  // namespace game_engine