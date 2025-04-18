#ifndef GAME_ENGINE_FRAMEBUFFER_H
#define GAME_ENGINE_FRAMEBUFFER_H

#include "gfx/rhi/rhi_new/common/rhi_types.h"

namespace game_engine {
namespace gfx {
namespace rhi {

/**
 * @brief Abstract base class for framebuffer implementations
 *
 * A framebuffer represents the actual textures that will be rendered into
 * during a render pass. It contains references to color and depth/stencil
 * textures that will be used as rendering targets.
 */
class Framebuffer {
  public:
  virtual ~Framebuffer() = default;

  virtual uint32_t getWidth() const = 0;

  virtual uint32_t getHeight() const = 0;

  virtual uint32_t getColorAttachmentCount() const = 0;

  virtual bool hasDSV() const = 0;
};

}  // namespace rhi
}  // namespace gfx
}  // namespace game_engine

#endif  // GAME_ENGINE_FRAMEBUFFER_H