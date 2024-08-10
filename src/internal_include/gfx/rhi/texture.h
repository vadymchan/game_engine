#ifndef GAME_ENGINE_TEXTURE_H
#define GAME_ENGINE_TEXTURE_H

#include "gfx/rhi/rhi_type.h"
#include "gfx/rhi/shader_bindable_resource.h"

#include <math_library/dimension.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <memory>

namespace game_engine {

struct jTexture : public ShaderBindableResource {
  jTexture()
      : type(ETextureType::MAX)
      , format(ETextureFormat::RGB8)
      , extent(0, 0)
      , mipLevels(1)
      , layerCount(1)
      , sampleCount(EMSAASamples::COUNT_1)
      , sRGB(false) {}

  jTexture(ETextureType              type,
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

  virtual ~jTexture() {}

  static int32_t GetMipLevels(int32_t InWidth, int32_t InHeight) {
    return 1
         + static_cast<uint32_t>(
               // TODO: instead of using (std::max) consider undefine max macro
               // from Window.h
               std::floor(std::log2((std::max)(InWidth, InHeight))));
  }

  virtual void* GetHandle() const { return nullptr; }

  virtual void* GetSamplerStateHandle() const { return nullptr; }

  virtual void Release() {}

  virtual EResourceLayout GetLayout() const {
    return EResourceLayout::UNDEFINED;
  }

  bool IsDepthFormat() const { return game_engine::IsDepthFormat(format); }

  bool IsDepthOnlyFormat() const {
    return game_engine::IsDepthOnlyFormat(format);
  }

  ETextureType       type;  // TODO: analog to VkImageViewType imageViewType
  ETextureFormat     format;
  math::Dimension2Di extent;
  uint32_t           mipLevels;
  uint32_t           layerCount;
  EMSAASamples       sampleCount;
  bool               sRGB;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_TEXTURE_H