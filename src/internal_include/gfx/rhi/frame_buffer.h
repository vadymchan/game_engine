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
  // ======= BEGIN: public getters ============================================

  size_t getHash() const {
    return GETHASH_FROM_INSTANT_STRUCT(m_textureType_,
                                       m_format_,
                                       m_extent_.width(),
                                       m_extent_.height(),
                                       m_layerCount_,
                                       m_isGenerateMipmap,
                                       m_sampleCount_);
  }

  // ======= END: public getters   ============================================

  // ======= BEGIN: public misc fields ========================================

  ETextureType       m_textureType_     = ETextureType::TEXTURE_2D;
  ETextureFormat     m_format_          = ETextureFormat::RGB8;
  math::Dimension2Di m_extent_          = math::Dimension2Di(0);
  int32_t            m_layerCount_      = 1;
  bool               m_isGenerateMipmap = false;
  int32_t            m_sampleCount_     = 1;

  // ======= END: public misc fields   ========================================
};

struct FrameBuffer : public std::enable_shared_from_this<FrameBuffer> {
  // ======= BEGIN: public destructor =========================================

  virtual ~FrameBuffer() {}

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public misc fields ========================================

  FrameBufferInfo                        m_info_;
  std::vector<std::shared_ptr<Texture> > m_textures_;
  std::shared_ptr<Texture>               m_textureDepth_;

  // ======= END: public misc fields   ========================================
};
}  // namespace game_engine

#endif  // GAME_ENGINE_FRAME_BUFFER_H
