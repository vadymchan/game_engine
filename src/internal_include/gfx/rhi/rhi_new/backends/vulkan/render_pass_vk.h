#ifndef GAME_ENGINE_RENDER_PASS_VK_H
#define GAME_ENGINE_RENDER_PASS_VK_H

#include "gfx/rhi/rhi_new/interface/render_pass.h"

#include <vulkan/vulkan.h>

#include <vector>

namespace game_engine {
namespace gfx {
namespace rhi {

class DeviceVk;

/**
 * Encapsulates a VkRenderPass object, which defines the attachments, subpasses,
 * and dependencies for a set of rendering operations.
 *
 * This corresponds directly to Vulkan's concept of render passes,
 * which makes rendering more efficient, especially on tile-based GPU architectures.
 */
class RenderPassVk : public RenderPass {
  public:
  RenderPassVk(const RenderPassDesc& desc, DeviceVk* device);
  ~RenderPassVk();

  VkRenderPass getRenderPass() const { return m_renderPass_; }

  bool shouldClearColor(uint32_t attachmentIndex) const override;
  bool shouldClearDepthStencil() const override;
  bool shouldClearStencil() const override;

  private:
  bool initialize_(const RenderPassDesc& desc);

  DeviceVk*    m_device_     = nullptr;
  VkRenderPass m_renderPass_ = VK_NULL_HANDLE;

  // Store info about load operations for implementation of shouldClear* methods
  std::vector<AttachmentLoadStoreOp> m_colorAttachmentOps_;
  AttachmentLoadStoreOp              m_depthStencilAttachmentOp_ = AttachmentLoadStoreOp::DontcareDontcare;
  bool                               m_hasDepthStencil_          = false;
};

}  // namespace rhi
}  // namespace gfx
}  // namespace game_engine

#endif  // GAME_ENGINE_RENDER_PASS_VK_H