#ifndef ARISE_TEXTURE_VK_H
#define ARISE_TEXTURE_VK_H

#include "gfx/rhi/backends/vulkan/rhi_enums_vk.h"
#include "gfx/rhi/interface/texture.h"

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace arise {
namespace gfx {
namespace rhi {

class DeviceVk;

class TextureVk : public Texture {
  public:
  TextureVk(const TextureDesc& desc, DeviceVk* device);

  /**
   * Create a texture wrapping an existing VkImage (for swap chain images)
   */
  TextureVk(DeviceVk* device, const TextureDesc& desc, VkImage existingImage, VkImageView existingImageView);

  ~TextureVk() override;

  TextureVk(const TextureVk&)            = delete;
  TextureVk& operator=(const TextureVk&) = delete;

  void transitionToInitialLayout(CommandBuffer* cmdBuffer);

  // Vulkan-specific methods
  VkImage getImage() const { return m_image_; }

  VkImageView getImageView() const { return m_imageView_; }

  VkFormat getVkFormat() const { return m_vkFormat_; }

  VkImageLayout getImageLayout() const { return g_getImageLayoutVk(m_currentLayout_); }

  // Updates texture data (for staging uploads)
  void update(const void* data, size_t dataSize, uint32_t mipLevel = 0, uint32_t arrayLayer = 0);

  private:
  friend class CommandBufferVk;
  friend class FramebufferVk;
  // Only CommandBufferVk and FrameBufferVk should update layouts through barriers
  void updateCurrentLayout_(ResourceLayout layout);

  bool createImage_();
  bool createImageView_();

  DeviceVk* m_device_;

  // Vulkan resources
  VkImage           m_image_      = VK_NULL_HANDLE;
  VmaAllocation     m_allocation_ = VK_NULL_HANDLE;
  VmaAllocationInfo m_allocationInfo_{};
  VkImageView       m_imageView_ = VK_NULL_HANDLE;

  // Track if we own these resources (false for swapchain images)
  bool m_ownsResources_ = true;

  VkFormat m_vkFormat_ = VK_FORMAT_UNDEFINED;
};

}  // namespace rhi
}  // namespace gfx
}  // namespace arise

#endif  // ARISE_TEXTURE_VK_H