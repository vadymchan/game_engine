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

  virtual void release() = 0;

  virtual void* getHandle() const {
    return m_TexturePtr_ ? m_TexturePtr_->getHandle() : nullptr;
  }

  std::shared_ptr<Texture> m_TexturePtr_;
};

class Swapchain {
  public:
  virtual ~Swapchain() {}

  virtual bool           create(const std::shared_ptr<Window>& window)     = 0;
  virtual void           release()                                         = 0;
  virtual void*          getHandle() const                                 = 0;
  virtual ETextureFormat getFormat() const                                 = 0;
  virtual const math::Dimension2Di& getExtent() const                      = 0;
  virtual SwapchainImage*           getSwapchainImage(int32_t index) const = 0;
  virtual int32_t                   getNumOfSwapchainImages() const        = 0;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_SWAPCHAIN_H