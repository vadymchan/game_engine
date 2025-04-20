#ifndef GAME_ENGINE_RENDER_PASS_DX12_H
#define GAME_ENGINE_RENDER_PASS_DX12_H

#include "gfx/rhi/interface/render_pass.h"
#include "platform/windows/windows_platform_setup.h"

#include <vector>

namespace game_engine {
namespace gfx {
namespace rhi {

class DeviceDx12;

/**
 * Unlike Vulkan, DirectX 12 traditionally doesn't have a first-class concept
 * of a render pass. This class emulates the behavior by tracking render target
 * and depth/stencil configurations, their load/store operations, and other
 * information needed for compatibility with the unified RHI interface.
 *
 * While DirectX 12 Ultimate does introduce a render pass API, we use the
 * traditional approach for broader compatibility and educational clarity.
 */
class RenderPassDx12 : public RenderPass {
  public:
  RenderPassDx12(const RenderPassDesc& desc, DeviceDx12* device);
  ~RenderPassDx12();

  const std::vector<DXGI_FORMAT>& getColorFormats() const { return m_colorFormats_; }

  DXGI_FORMAT getDepthStencilFormat() const { return m_depthStencilFormat_; }

  bool shouldClearColor(uint32_t attachmentIndex) const override;
  bool shouldClearDepthStencil() const override;
  bool shouldClearStencil() const override;

  private:
  bool initialize_(const RenderPassDesc& desc);

  DeviceDx12* m_device_ = nullptr;

  // In DX12 we don't have a render pass object, just store formats and operations
  std::vector<DXGI_FORMAT> m_colorFormats_;
  DXGI_FORMAT              m_depthStencilFormat_ = DXGI_FORMAT_UNKNOWN;

  std::vector<AttachmentLoadStoreOp> m_colorAttachmentOps_;
  AttachmentLoadStoreOp              m_depthStencilAttachmentOp_ = AttachmentLoadStoreOp::DontcareDontcare;
  bool                               m_hasDepthStencil_          = false;
};

}  // namespace rhi
}  // namespace gfx
}  // namespace game_engine

#endif  // GAME_ENGINE_RENDER_PASS_DX12_H