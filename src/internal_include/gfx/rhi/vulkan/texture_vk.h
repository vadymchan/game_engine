#ifndef GAME_ENGINE_TEXTURE_VK_H
#define GAME_ENGINE_TEXTURE_VK_H

#include "gfx/rhi/texture.h"
#include "gfx/rhi/vulkan/utils_vk.h"

#include <math_library/dimension.h>
#include <vulkan/vulkan.h>

#include <cmath>

namespace game_engine {

class TextureVk : public Texture {
  public:
  TextureVk()
      : Texture() {}

  // TODO: constructor is not complete
  TextureVk(ETextureType              type,
            ETextureFormat            format,
            const math::Dimension2Di& extent,
            uint32_t                  layerCount  = 1,
            EMSAASamples              sampleCount = EMSAASamples::COUNT_1,
            bool                      sRGB      = false)
      : Texture(type, format, extent, layerCount, sampleCount, sRGB) {}

  virtual ~TextureVk() { ReleaseInternal(); }

  virtual void Release() override { ReleaseInternal(); }

  void ReleaseInternal();

  virtual void* GetHandle() const override { return m_image_; }

  virtual void* GetSamplerStateHandle() const override { return m_sampler_; }

  virtual EResourceLayout GetLayout() const override { return m_imageLayout_; }

  // TODO: clear resources

  static VkSampler CreateDefaultSamplerState();
  static void      DestroyDefaultSamplerState();

  // private:
  VkImage     m_image_ = VK_NULL_HANDLE;  // TODO: consider using pool
  VkImageView m_imageView_
      = VK_NULL_HANDLE;  // TODO: consider several image views per VkImage
  VkSampler       m_sampler_      = VK_NULL_HANDLE;
  EResourceLayout m_imageLayout_  = EResourceLayout::UNDEFINED;
  VkDeviceMemory  m_deviceMemory_ = VK_NULL_HANDLE;

  // TODO:
  // - UAV image view
  // - std::map for each mip map image view
};

}  // namespace game_engine

#endif  // GAME_ENGINE_TEXTURE_VK_H
