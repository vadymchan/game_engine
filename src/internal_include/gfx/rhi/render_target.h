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

struct jRenderTargetInfo {
  size_t GetHash() const {
    return GETHASH_FROM_INSTANT_STRUCT(Type,
                                       Format,
                                       Extent.width(),
                                       Extent.height(),
                                       LayerCount,
                                       IsGenerateMipmap,
                                       SampleCount,
                                       RTClearValue.GetHash(),
                                       TextureCreateFlag,
                                       IsUseAsSubpassInput,
                                       IsMemoryless);
  }

  ETextureType       Type                = ETextureType::TEXTURE_2D;
  ETextureFormat     Format              = ETextureFormat::RGB8;
  math::Dimension2Di Extent              = math::Dimension2Di(0);
  int32_t            LayerCount          = 1;
  bool               IsGenerateMipmap    = false;
  EMSAASamples       SampleCount         = EMSAASamples::COUNT_1;
  bool               IsUseAsSubpassInput = false;
  bool               IsMemoryless        = false;
  jRTClearValue      RTClearValue        = jRTClearValue::Invalid;
  ETextureCreateFlag TextureCreateFlag   = ETextureCreateFlag::RTV;
};

struct jRenderTarget final
    : public std::enable_shared_from_this<jRenderTarget> {
  // Create render target from texture, It is useful to create render target
  // from swapchain texture
  template <typename T1, class... T2>
  static std::shared_ptr<jRenderTarget> CreateFromTexture(const T2&... args) {
    const auto& T1Ptr = std::make_shared<T1>(args...);
    return std::make_shared<jRenderTarget>(T1Ptr);
  }

  static std::shared_ptr<jRenderTarget> CreateFromTexture(
      const std::shared_ptr<jTexture>& texturePtr) {
    return std::make_shared<jRenderTarget>(texturePtr);
  }

  jRenderTarget() = default;

  jRenderTarget(const std::shared_ptr<jTexture>& InTexturePtr)
      : TexturePtr(InTexturePtr) {
    if (InTexturePtr) {
      Info.Type        = InTexturePtr->type;
      Info.Format      = InTexturePtr->format;
      Info.Extent      = InTexturePtr->extent;
      Info.LayerCount  = InTexturePtr->layerCount;
      Info.SampleCount = InTexturePtr->sampleCount;
    }
  }

  ~jRenderTarget() {}

  size_t GetHash() const {
    if (Hash) {
      return Hash;
    }

    Hash = Info.GetHash();
    if (GetTexture()) {
      Hash = XXH64(reinterpret_cast<uint64_t>(GetTexture()->GetHandle()), Hash);
    }
    return Hash;
  }

  void Return();

  EResourceLayout GetLayout() const {
    return TexturePtr ? TexturePtr->GetLayout() : EResourceLayout::UNDEFINED;
  }

  jTexture* GetTexture() const { return TexturePtr.get(); }

  jRenderTargetInfo         Info;
  std::shared_ptr<jTexture> TexturePtr;

  mutable size_t Hash                         = 0;
  bool           bCreatedFromRenderTargetPool = false;
};

// ==================== SceneRenderTarget ( for Renderer) =====================

struct SceneRenderTarget {
  std::shared_ptr<jRenderTarget> ColorPtr;
  std::shared_ptr<jRenderTarget> DepthPtr;
  std::shared_ptr<jRenderTarget> ResolvePtr;

  // Final rendered image, post-processed
  std::shared_ptr<jRenderTarget> FinalColorPtr;

  void Create(std::shared_ptr<Window> window,
              const jSwapchainImage*  InSwapchain);

  void Return();
};

}  // namespace game_engine

#endif  // GAME_ENGINE_RENDER_TARGET_H