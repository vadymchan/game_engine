#ifndef GAME_ENGINE_RENDER_TARGET_H
#define GAME_ENGINE_RENDER_TARGET_H

#include "gfx/rhi/rhi_type.h"
#include "gfx/rhi/swapchain.h"
#include "gfx/rhi/texture.h"
#include "platform/common/window.h"

#include <math_library/dimension.h>

#include <cstdint>
#include <memory>

namespace game_engine {

struct RenderTargetInfo {
  // ======= BEGIN: public getters ============================================

  size_t getHash() const {
    return GETHASH_FROM_INSTANT_STRUCT(m_rype_,
                                       m_format_,
                                       m_extent_.width(),
                                       m_extent_.height(),
                                       m_layerCount_,
                                       m_isGenerateMipmap_,
                                       m_sampleCount_,
                                       m_rtClearValue.getHash(),
                                       m_textureCreateFlag_,
                                       m_isUseAsSubpassInput_,
                                       m_isMemoryless_);
  }

  // ======= END: public getters   ============================================

  // ======= BEGIN: public misc fields ========================================

  ETextureType       m_rype_                = ETextureType::TEXTURE_2D;
  ETextureFormat     m_format_              = ETextureFormat::RGB8;
  math::Dimension2Di m_extent_              = math::Dimension2Di(0);
  int32_t            m_layerCount_          = 1;
  bool               m_isGenerateMipmap_    = false;
  EMSAASamples       m_sampleCount_         = EMSAASamples::COUNT_1;
  bool               m_isUseAsSubpassInput_ = false;
  bool               m_isMemoryless_        = false;
  RtClearValue       m_rtClearValue         = RtClearValue::s_kInvalid;
  ETextureCreateFlag m_textureCreateFlag_   = ETextureCreateFlag::RTV;

  // ======= END: public misc fields   ========================================
};

struct RenderTarget final : public std::enable_shared_from_this<RenderTarget> {
  // ======= BEGIN: public static methods =====================================

  // Create render target from texture, It is useful to create render target
  // from swapchain texture
  template <typename T1, class... T2>
  static std::shared_ptr<RenderTarget> s_createFromTexture(const T2&... args) {
    const auto& T1Ptr = std::make_shared<T1>(args...);
    return std::make_shared<RenderTarget>(T1Ptr);
  }

  static std::shared_ptr<RenderTarget> s_createFromTexture(
      const std::shared_ptr<Texture>& texturePtr) {
    return std::make_shared<RenderTarget>(texturePtr);
  }

  // ======= END: public static methods   =====================================

  // ======= BEGIN: public constructors =======================================

  RenderTarget() = default;

  RenderTarget(const std::shared_ptr<Texture>& texturePtr)
      : m_texture_(texturePtr) {
    if (texturePtr) {
      m_info_.m_rype_        = texturePtr->m_type_;
      m_info_.m_format_      = texturePtr->m_format_;
      m_info_.m_extent_      = texturePtr->m_extent_;
      m_info_.m_layerCount_  = texturePtr->m_layerCount_;
      m_info_.m_sampleCount_ = texturePtr->m_sampleCount_;
    }
  }

  // ======= END: public constructors   =======================================

  // ======= BEGIN: public destructor =========================================

  ~RenderTarget() {}

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public getters ============================================

  size_t getHash() const {
    if (m_hash_) {
      return m_hash_;
    }

    m_hash_ = m_info_.getHash();
    if (getTexture()) {
      m_hash_ = XXH64(reinterpret_cast<uint64_t>(getTexture()->getHandle()),
                      m_hash_);
    }
    return m_hash_;
  }

  EResourceLayout getLayout() const {
    return m_texture_ ? m_texture_->getLayout() : EResourceLayout::UNDEFINED;
  }

  std::shared_ptr<Texture> getTexture() const { return m_texture_; }

  // ======= END: public getters   ============================================

  // ======= BEGIN: public misc methods =======================================

  // TODO: consider renaming
  void returnRt();

  // ======= END: public misc methods   =======================================

  // ======= BEGIN: public misc fields ========================================

  RenderTargetInfo         m_info_;
  std::shared_ptr<Texture> m_texture_;

  mutable size_t m_hash_                          = 0;
  bool           m_isCreatedFromRenderTargetPool_ = false;

  // ======= END: public misc fields   ========================================
};

// ==================== SceneRenderTarget ( for Renderer) =====================

struct SceneRenderTarget {
  // ======= BEGIN: public misc methods =======================================

  void create(math::Dimension2Di                      dimensions,
              const std::shared_ptr<ISwapchainImage>& swapchain);

  void returnRt();

  // ======= END: public misc methods   =======================================

  // ======= BEGIN: public misc fields ========================================

  // ======= END: public misc fields   ========================================
  std::shared_ptr<RenderTarget> m_colorBuffer_;
  std::shared_ptr<RenderTarget> m_depthBuffer_;
  // std::shared_ptr<RenderTarget> m_resolvePtr_;

  // Final rendered image, post-processed
  std::shared_ptr<RenderTarget> m_backBuffer_;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_RENDER_TARGET_H
