#ifndef GAME_ENGINE_TEXTURE_VK_H
#define GAME_ENGINE_TEXTURE_VK_H

#include "gfx/rhi/rhi_new/interface/texture.h"

#include <vulkan/vulkan.h>

namespace game_engine {
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

  // Vulkan-specific methods
  VkImage getImage() const { return m_image_; }

  VkImageView getImageView() const { return m_imageView_; }

  VkFormat getVkFormat() const { return m_vkFormat_; }

  VkImageLayout getCurrentLayout() const { return m_vkLayout_; }

  // Updates texture data (for staging uploads)
  void update(const void* data, size_t dataSize, uint32_t mipLevel = 0, uint32_t arrayLayer = 0);

  private:
  friend class CommandBufferVk;
  // Only CommandBufferVk should update layouts through barriers
  void updateCurrentLayout_(VkImageLayout layout);

  bool createImage_();
  bool allocateMemory_();
  bool createImageView_();

  DeviceVk* m_device_;

  // Vulkan resources
  VkImage        m_image_     = VK_NULL_HANDLE;
  VkDeviceMemory m_memory_    = VK_NULL_HANDLE;
  VkImageView    m_imageView_ = VK_NULL_HANDLE;

  // Track if we own these resources (false for swapchain images)
  bool m_ownsImage_ = true;

  // Vulkan-specific format and layout tracking
  VkFormat      m_vkFormat_ = VK_FORMAT_UNDEFINED;
  VkImageLayout m_vkLayout_ = VK_IMAGE_LAYOUT_UNDEFINED;
};

}  // namespace rhi
}  // namespace gfx
}  // namespace game_engine

#endif  // GAME_ENGINE_TEXTURE_VK_H