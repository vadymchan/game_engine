#ifndef GAME_ENGINE_TEXTURE_VK_H
#define GAME_ENGINE_TEXTURE_VK_H

#include "gfx/rhi/vulkan/utils_vk.h"

#include <math_library/dimension.h>
#include <vulkan/vulkan.h>

#include <cmath>

namespace game_engine {

static VkSampler g_defaultSampler = nullptr;

class TextureVk {
  public:
  TextureVk()
      : type(ETextureType::MAX)
      , format(ETextureFormat::RGB8)
      , extent(0, 0)
      , mipLevels(1)
      , layerCount(1)
      , sampleCount(EMSAASamples::COUNT_1)
      , sRGB(false) {}

  // TODO: constructor is not complete
  TextureVk(ETextureType              type,
            ETextureFormat            format,
            const math::Dimension2Di& extent,
            uint32_t                  layerCount  = 1,
            EMSAASamples              sampleCount = EMSAASamples::COUNT_1,
            bool                      InSRGB      = false)
      : type(type)
      , format(format)
      , extent(extent)
      , mipLevels(GetMipLevels(extent.width(), extent.height()))
      , layerCount(layerCount)
      , sampleCount(sampleCount)
      , sRGB(InSRGB) {}

  virtual ~TextureVk() { ReleaseInternal(); }

  virtual void Release() { ReleaseInternal(); }

  void ReleaseInternal();

  static uint32_t GetMipLevels(uint32_t InWidth, uint32_t InHeight);

  virtual EResourceLayout GetLayout() const { return imageLayout; }

  // TODO: clear resources

  bool IsDepthFormat() const { return game_engine::IsDepthFormat(format); }

  bool IsDepthOnlyFormat() const {
    return game_engine::IsDepthOnlyFormat(format);
  }

  static VkSampler CreateDefaultSamplerState();

  // private:
  VkImage     image = VK_NULL_HANDLE;  // TODO: consider using pool
  VkImageView imageView
      = VK_NULL_HANDLE;     // TODO: consider several image views per VkImage
  ETextureType       type;  // TODO: analog to VkImageViewType imageViewType
  VkSampler          sampler = VK_NULL_HANDLE;
  ETextureFormat     format;
  math::Dimension2Di extent;
  bool               sRGB;
  uint32_t           mipLevels;
  uint32_t           layerCount;
  EMSAASamples       sampleCount;
  EResourceLayout    imageLayout = EResourceLayout::UNDEFINED;
  VkDeviceMemory     deviceMemory = VK_NULL_HANDLE;

  // TODO:
  // - UAV image view
  // - std::map for each mip map image view
};

}  // namespace game_engine

#endif  // GAME_ENGINE_TEXTURE_VK_H
