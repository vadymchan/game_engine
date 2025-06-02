#ifndef GAME_ENGINE_FRAMEBUFFER_DX12_H
#define GAME_ENGINE_FRAMEBUFFER_DX12_H

#include "gfx/rhi/interface/framebuffer.h"
#include "platform/windows/windows_platform_setup.h"

#include <vector>

namespace game_engine {
namespace gfx {
namespace rhi {

class DeviceDx12;
class TextureDx12;
class CommandBufferDx12;
class RenderPassDx12;

/**
 * DirectX 12 doesn't have a direct equivalent to Vulkan's framebuffer concept.
 * Instead, render targets are bound directly to the command list through
 * OMSetRenderTargets.
 *
 * This class encapsulates the necessary render target views, depth/stencil views,
 * and metadata needed to emulate the Vulkan-style framebuffer in DirectX 12.
 */
class FramebufferDx12 : public Framebuffer {
  public:
  FramebufferDx12(const FramebufferDesc& desc, DeviceDx12* device);
  ~FramebufferDx12() = default;

  uint32_t getWidth() const override { return m_width_; }

  uint32_t getHeight() const override { return m_height_; }

  uint32_t getColorAttachmentCount() const override { return static_cast<uint32_t>(m_colorAttachments_.size()); }

  bool hasDSV() const override { return m_hasDepthStencil_; }

  // DX12-specific methods
  const D3D12_CPU_DESCRIPTOR_HANDLE* getRTVHandles() const { return m_rtvHandles_.data(); }

  const D3D12_CPU_DESCRIPTOR_HANDLE* getDsvHandle() const { return m_hasDepthStencil_ ? &m_dsvHandle_ : nullptr; }

  // Helper methods for DX12 command buffer
  void transitionToRenderTargetState(CommandBufferDx12* cmdBuffer);
  void transitionToResourceState(CommandBufferDx12* cmdBuffer, RenderPassDx12* renderPass);

  private:
  bool initialize_(const FramebufferDesc& desc);

  DeviceDx12* m_device_ = nullptr;

  std::vector<TextureDx12*> m_colorAttachments_;
  TextureDx12*              m_depthStencilAttachment_ = nullptr;

  std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> m_rtvHandles_;
  D3D12_CPU_DESCRIPTOR_HANDLE              m_dsvHandle_ = {};

  uint32_t m_width_           = 0;
  uint32_t m_height_          = 0;
  bool     m_hasDepthStencil_ = false;
};

}  // namespace rhi
}  // namespace gfx
}  // namespace game_engine

#endif  // GAME_ENGINE_FRAMEBUFFER_DX12_H