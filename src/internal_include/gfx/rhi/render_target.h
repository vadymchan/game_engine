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
  size_t GetHash() const {
    return GETHASH_FROM_INSTANT_STRUCT(m_rype_,
                                       m_format_,
                                       m_extent_.width(),
                                       m_extent_.height(),
                                       m_layerCount_,
                                       m_isGenerateMipmap_,
                                       m_sampleCount_,
                                       m_rtClearValue.GetHash(),
                                       m_textureCreateFlag_,
                                       m_isUseAsSubpassInput_,
                                       m_isMemoryless_);
  }

  ETextureType       m_rype_                = ETextureType::TEXTURE_2D;
  ETextureFormat     m_format_              = ETextureFormat::RGB8;
  math::Dimension2Di m_extent_              = math::Dimension2Di(0);
  int32_t            m_layerCount_          = 1;
  bool               m_isGenerateMipmap_    = false;
  EMSAASamples       m_sampleCount_         = EMSAASamples::COUNT_1;
  bool               m_isUseAsSubpassInput_ = false;
  bool               m_isMemoryless_        = false;
  RTClearValue       m_rtClearValue         = RTClearValue::s_kInvalid;
  ETextureCreateFlag m_textureCreateFlag_   = ETextureCreateFlag::RTV;
};

struct RenderTarget final : public std::enable_shared_from_this<RenderTarget> {
  // Create render target from texture, It is useful to create render target
  // from swapchain texture
  template <typename T1, class... T2>
  static std::shared_ptr<RenderTarget> CreateFromTexture(const T2&... args) {
    const auto& T1Ptr = std::make_shared<T1>(args...);
    return std::make_shared<RenderTarget>(T1Ptr);
  }

  static std::shared_ptr<RenderTarget> CreateFromTexture(
      const std::shared_ptr<Texture>& texturePtr) {
    return std::make_shared<RenderTarget>(texturePtr);
  }

  RenderTarget() = default;

  RenderTarget(const std::shared_ptr<Texture>& InTexturePtr)
      : m_texturePtr_(InTexturePtr) {
    if (InTexturePtr) {
      m_info_.m_rype_        = InTexturePtr->m_type_;
      m_info_.m_format_      = InTexturePtr->m_format_;
      m_info_.m_extent_      = InTexturePtr->m_extent_;
      m_info_.m_layerCount_  = InTexturePtr->m_layerCount_;
      m_info_.m_sampleCount_ = InTexturePtr->m_sampleCount_;
    }
  }

  ~RenderTarget() {}

  size_t GetHash() const {
    if (m_hash_) {
      return m_hash_;
    }

    m_hash_ = m_info_.GetHash();
    if (GetTexture()) {
      m_hash_ = XXH64(reinterpret_cast<uint64_t>(GetTexture()->GetHandle()), m_hash_);
    }
    return m_hash_;
  }

  void Return();

  EResourceLayout GetLayout() const {
    return m_texturePtr_ ? m_texturePtr_->GetLayout() : EResourceLayout::UNDEFINED;
  }

  Texture* GetTexture() const { return m_texturePtr_.get(); }

  RenderTargetInfo         m_info_;
  std::shared_ptr<Texture> m_texturePtr_;

  mutable size_t m_hash_                         = 0;
  bool           m_isCreatedFromRenderTargetPool_ = false;
};

// ==================== SceneRenderTarget ( for Renderer) =====================

struct SceneRenderTarget {
  std::shared_ptr<RenderTarget> m_colorPtr_;
  std::shared_ptr<RenderTarget> m_depthPtr_;
  std::shared_ptr<RenderTarget> m_resolvePtr_;

  // Final rendered image, post-processed
  std::shared_ptr<RenderTarget> m_finalColorPtr_;

  void Create(std::shared_ptr<Window> window,
              const SwapchainImage*   InSwapchain);

  void Return();
};

}  // namespace game_engine

#endif  // GAME_ENGINE_RENDER_TARGET_H