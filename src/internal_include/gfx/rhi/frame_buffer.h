#ifndef GAME_ENGINE_FRAME_BUFFER_H
#define GAME_ENGINE_FRAME_BUFFER_H

#include "gfx/rhi/instant_struct.h"
#include "gfx/rhi/rhi_type.h"
#include "gfx/rhi/texture.h"

#include <math_library/dimension.h>

#include <cstdint>
#include <memory>
#include <vector>

namespace game_engine {

struct FrameBufferInfo {

  size_t GetHash() const {
    return GETHASH_FROM_INSTANT_STRUCT(m_textureType_,
                                       m_format_,
                                       m_extent_.width(),
                                       m_extent_.height(),
                                       m_layerCount_,
                                       m_isGenerateMipmap,
                                       m_sampleCount_);
  }

  ETextureType       m_textureType_      = ETextureType::TEXTURE_2D;
  ETextureFormat     m_format_           = ETextureFormat::RGB8;
  math::Dimension2Di m_extent_           = math::Dimension2Di(0);
  int32_t            m_layerCount_       = 1;
  bool               m_isGenerateMipmap = false;
  int32_t            m_sampleCount_      = 1;
};

struct FrameBuffer : public std::enable_shared_from_this<FrameBuffer> {
  virtual ~FrameBuffer() {}

  // No need for now
  //virtual Texture* GetTexture(int32_t index = 0) const {
  //  return Textures[index].get();
  //}

  //virtual Texture* GetTextureDepth(int32_t index = 0) const {
  //  return TextureDepth.get();
  //}

  //virtual ETextureType GetTextureType() const { return Info.TextureType; }

  //virtual bool SetDepthAttachment(const std::shared_ptr<Texture>& InDepth) {
  //  TextureDepth = InDepth;
  //  return true;
  //}

  //virtual void SetDepthMipLevel(int32_t InLevel) {}

  //virtual bool FBOBegin(int index = 0, bool mrt = false) const { return true; }

  //virtual void End() const {}

  FrameBufferInfo                        m_info_;
  std::vector<std::shared_ptr<Texture> > m_textures_;
  std::shared_ptr<Texture>               m_textureDepth_;
};
}  // namespace game_engine

#endif  // GAME_ENGINE_FRAME_BUFFER_H