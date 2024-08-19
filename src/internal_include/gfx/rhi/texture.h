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

struct Texture : public ShaderBindableResource {
  Texture()
      : m_type_(ETextureType::MAX)
      , m_format_(ETextureFormat::RGB8)
      , m_extent_(0, 0)
      , m_mipLevels_(1)
      , m_layerCount_(1)
      , m_sampleCount_(EMSAASamples::COUNT_1)
      , m_sRGB_(false) {}

  Texture(ETextureType              type,
           ETextureFormat            format,
           const math::Dimension2Di& extent,
           uint32_t                  layerCount  = 1,
           EMSAASamples              sampleCount = EMSAASamples::COUNT_1,
           bool                      sRGB      = false)
      : m_type_(type)
      , m_format_(format)
      , m_extent_(extent)
      , m_mipLevels_(GetMipLevels(extent.width(), extent.height()))
      , m_layerCount_(layerCount)
      , m_sampleCount_(sampleCount)
      , m_sRGB_(sRGB) {}

  virtual ~Texture() {}

  static int32_t GetMipLevels(int32_t witdh, int32_t height) {
    return 1
         + static_cast<uint32_t>(
               // TODO: instead of using (std::max) consider undefine max macro
               // from Window.h
               std::floor(std::log2((std::max)(witdh, height))));
  }

  virtual void* GetHandle() const { return nullptr; }

  virtual void* GetSamplerStateHandle() const { return nullptr; }

  virtual void Release() {}

  virtual EResourceLayout GetLayout() const {
    return EResourceLayout::UNDEFINED;
  }

  bool IsDepthFormat() const { return game_engine::IsDepthFormat(m_format_); }

  bool IsDepthOnlyFormat() const {
    return game_engine::IsDepthOnlyFormat(m_format_);
  }

  ETextureType       m_type_;  // TODO: analog to VkImageViewType imageViewType
  ETextureFormat     m_format_;
  math::Dimension2Di m_extent_;
  uint32_t           m_mipLevels_;
  uint32_t           m_layerCount_;
  EMSAASamples       m_sampleCount_;
  bool               m_sRGB_;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_TEXTURE_H