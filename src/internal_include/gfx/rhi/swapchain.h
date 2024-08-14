#ifndef GAME_ENGINE_SWAPCHAIN_H
#define GAME_ENGINE_SWAPCHAIN_H

#include "gfx/rhi/rhi_type.h"
#include "gfx/rhi/texture.h"
#include "platform/common/window.h"

#include <math_library/vector.h>

#include <cstdint>
#include <memory>

namespace game_engine {

class SwapchainImage {
  public:
  virtual ~SwapchainImage() {}

  virtual void Release() = 0;

  virtual void* GetHandle() const {
    return TexturePtr ? TexturePtr->GetHandle() : nullptr;
  }

  std::shared_ptr<Texture> TexturePtr;
};

class Swapchain {
  public:
  virtual ~Swapchain() {}

  virtual bool           Create(const std::shared_ptr<Window>& window)     = 0;
  virtual void           Release()                                         = 0;
  virtual void*          GetHandle() const                                 = 0;
  virtual ETextureFormat GetFormat() const                                 = 0;
  virtual const math::Dimension2Di& GetExtent() const                      = 0;
  virtual SwapchainImage*          GetSwapchainImage(int32_t index) const = 0;
  virtual int32_t                   GetNumOfSwapchainImages() const        = 0;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_SWAPCHAIN_H