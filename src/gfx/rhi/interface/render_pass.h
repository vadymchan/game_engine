#ifndef GAME_ENGINE_RENDER_PASS_H
#define GAME_ENGINE_RENDER_PASS_H

#include "gfx/rhi/common/rhi_types.h"

namespace game_engine {
namespace gfx {
namespace rhi {

/**
 * A render pass represents a sequence of rendering operations that operate on a set
 * of attachments. It defines which textures are used for rendering and how they should
 * be handled (loaded, cleared, stored, etc.)
 */
class RenderPass {
  public:
  virtual ~RenderPass() = default;

  /**
   * @brief Checks if a color attachment should be cleared at the beginning of the render pass
   * @param attachmentIndex Index of the color attachment
   */
  virtual bool shouldClearColor(uint32_t attachmentIndex) const = 0;

  virtual bool shouldClearDepthStencil() const = 0;

  virtual bool shouldClearStencil() const = 0;
};

}  // namespace rhi
}  // namespace gfx
}  // namespace game_engine

#endif  // GAME_ENGINE_RENDER_PASS_HS