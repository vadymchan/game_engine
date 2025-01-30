#ifndef GAME_ENGINE_SWAPCHAIN_H
#define GAME_ENGINE_SWAPCHAIN_H

#include "gfx/rhi/rhi_type.h"
#include "gfx/rhi/texture.h"
#include "platform/common/window.h"

#include <math_library/vector.h>

#include <cstdint>
#include <memory>

namespace game_engine {

class ISwapchainImage {
  public:
  // ======= BEGIN: public destructor =========================================

  virtual ~ISwapchainImage() {}

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overridden methods =================================

  virtual void release() = 0;

  virtual void* getHandle() const {
    return m_texture_ ? m_texture_->getHandle() : nullptr;
  }

  // ======= END: public overridden methods   =================================

  // ======= BEGIN: public misc fields ========================================

  std::shared_ptr<Texture> m_texture_;

  // ======= END: public misc fields   ========================================
};

class ISwapchain {
  public:
  // ======= BEGIN: public destructor =========================================

  virtual ~ISwapchain() {}

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overridden methods =================================

  virtual bool           create(const std::shared_ptr<Window>& window) = 0;
  virtual void           release()                                     = 0;
  virtual void*          getHandle() const                             = 0;
  virtual ETextureFormat getFormat() const                             = 0;
  virtual const math::Dimension2Di&        getExtent() const           = 0;
  virtual std::shared_ptr<ISwapchainImage> getSwapchainImage(
      int32_t index) const
      = 0;
  virtual int32_t getNumOfSwapchainImages() const = 0;

  // ======= END: public overridden methods   =================================
};

}  // namespace game_engine

#endif  // GAME_ENGINE_SWAPCHAIN_H
