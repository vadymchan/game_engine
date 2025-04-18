#ifndef GAME_ENGINE_FRAMEBUFFER_VK_H
#define GAME_ENGINE_FRAMEBUFFER_VK_H

#include "gfx/rhi/rhi_new/interface/framebuffer.h"

#include <vulkan/vulkan.h>

#include <vector>

namespace game_engine {
namespace gfx {
namespace rhi {

class DeviceVk;
class TextureVk;
class RenderPassVk;

/**
 * Encapsulates a VkFramebuffer object, which is a collection of VkImageViews
 * that serve as the actual render targets during rendering. In Vulkan,
 * a framebuffer is used in conjunction with a render pass to define where
 * rendering output goes.
 */
class FramebufferVk : public Framebuffer {
  public:
  FramebufferVk(const FramebufferDesc& desc, DeviceVk* device);
  ~FramebufferVk();

  VkFramebuffer getFramebuffer() const { return m_framebuffer; }

  uint32_t getWidth() const override { return m_width; }

  uint32_t getHeight() const override { return m_height; }

  uint32_t getColorAttachmentCount() const override { return static_cast<uint32_t>(m_colorAttachments.size()); }

  bool hasDSV() const override { return m_hasDepthStencil; }

  private:
  bool initialize(const FramebufferDesc& desc);

  DeviceVk*     m_device      = nullptr;
  VkFramebuffer m_framebuffer = VK_NULL_HANDLE;

  std::vector<TextureVk*> m_colorAttachments;
  TextureVk*              m_depthStencilAttachment = nullptr;

  uint32_t m_width           = 0;
  uint32_t m_height          = 0;
  bool     m_hasDepthStencil = false;
};

}  // namespace rhi
}  // namespace gfx
}  // namespace game_engine

#endif  // GAME_ENGINE_FRAMEBUFFER_VK_H