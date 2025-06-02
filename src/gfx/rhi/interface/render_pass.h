#ifndef ARISE_RHI_RENDER_PASS_H
#define ARISE_RHI_RENDER_PASS_H

#include "gfx/rhi/common/rhi_types.h"

namespace arise {
namespace gfx {
namespace rhi {

/**
 * A render pass represents a sequence of rendering operations that operate on a set
 * of attachments. It defines which textures are used for rendering and how they should
 * be handled (loaded, cleared, stored, etc.)
 */
class RenderPass {
  public:
  RenderPass(const RenderPassDesc& desc)
      : m_desc_(desc) {}
  virtual ~RenderPass() = default;

  /**
   * @brief Checks if a color attachment should be cleared at the beginning of the render pass
   * @param attachmentIndex Index of the color attachment
   */
  virtual bool shouldClearColor(uint32_t attachmentIndex) const = 0;

  virtual bool shouldClearDepthStencil() const = 0;

  virtual bool shouldClearStencil() const = 0;

  protected:
  RenderPassDesc m_desc_;
};

}  // namespace rhi
}  // namespace gfx
}  // namespace arise

#endif  // ARISE_RHI_RENDER_PASS_HS