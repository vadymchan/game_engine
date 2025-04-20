#include "gfx/rhi/backends/dx12/framebuffer_dx12.h"

#include "gfx/rhi/backends/dx12/command_buffer_dx12.h"
#include "gfx/rhi/backends/dx12/device_dx12.h"
#include "gfx/rhi/backends/dx12/rhi_enums_dx12.h"
#include "gfx/rhi/backends/dx12/texture_dx12.h"
#include "utils/logger/global_logger.h"

namespace game_engine {
namespace gfx {
namespace rhi {

FramebufferDx12::FramebufferDx12(const FramebufferDesc& desc, DeviceDx12* device)
    : m_device_(device)
    , m_width_(desc.width)
    , m_height_(desc.height)
    , m_hasDepthStencil_(desc.hasDepthStencil) {
  if (!initialize_(desc)) {
    GlobalLogger::Log(LogLevel::Error, "Failed to initialize framebuffer");
  }
}

bool FramebufferDx12::initialize_(const FramebufferDesc& desc) {
  // Store color attachments
  m_colorAttachments_.clear();
  m_rtvHandles_.clear();

  for (auto* texture : desc.colorAttachments) {
    TextureDx12* textureDx12 = dynamic_cast<TextureDx12*>(texture);
    if (!textureDx12) {
      GlobalLogger::Log(LogLevel::Error, "Invalid texture type for color attachment");
      return false;
    }

    m_colorAttachments_.push_back(textureDx12);

    // Get RTV handle
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = textureDx12->getRtvHandle();
    if (rtvHandle.ptr == 0) {
      GlobalLogger::Log(LogLevel::Error, "Texture doesn't have a valid RTV");
      return false;
    }

    m_rtvHandles_.push_back(rtvHandle);
  }

  // Store depth/stencil attachment
  if (desc.hasDepthStencil && desc.depthStencilAttachment) {
    m_depthStencilAttachment_ = dynamic_cast<TextureDx12*>(desc.depthStencilAttachment);
    if (!m_depthStencilAttachment_) {
      GlobalLogger::Log(LogLevel::Error,
                        "Invalid texture type for depth/stencil attachment");
      return false;
    }

    // Get DSV handle
    m_dsvHandle_ = m_depthStencilAttachment_->getDsvHandle();
    if (m_dsvHandle_.ptr == 0) {
      GlobalLogger::Log(LogLevel::Error, "Texture doesn't have a valid DSV");
      return false;
    }
  }

  return true;
}

void FramebufferDx12::transitionToRenderTargetState(CommandBufferDx12* cmdBuffer) {
  // Transition color attachments to render target state
  for (auto* texture : m_colorAttachments_) {
    ResourceBarrierDesc barrier = {};
    barrier.texture             = texture;
    barrier.oldLayout           = texture->getCurrentLayoutType();  
    barrier.newLayout           = ResourceLayout::ColorAttachment;
    cmdBuffer->resourceBarrier(barrier);
  }

  // Transition depth/stencil attachment if present
  if (m_hasDepthStencil_ && m_depthStencilAttachment_) {
    ResourceBarrierDesc barrier = {};
    barrier.texture             = m_depthStencilAttachment_;
    barrier.oldLayout           = m_depthStencilAttachment_->getCurrentLayoutType();  
    barrier.newLayout           = ResourceLayout::DepthStencilAttachment;
    cmdBuffer->resourceBarrier(barrier);
  }
}

void FramebufferDx12::transitionToResourceState(CommandBufferDx12* cmdBuffer) {
  // Transition color attachments back to shader read state
  for (auto* texture : m_colorAttachments_) {
    ResourceBarrierDesc barrier = {};
    barrier.texture             = texture;
    barrier.oldLayout           = texture->getCurrentLayoutType();
    barrier.newLayout           = ResourceLayout::ShaderReadOnly;
    cmdBuffer->resourceBarrier(barrier);
  }

  // Transition depth/stencil attachment if present
  if (m_hasDepthStencil_ && m_depthStencilAttachment_) {
    ResourceBarrierDesc barrier = {};
    barrier.texture             = m_depthStencilAttachment_;
    barrier.oldLayout           = m_depthStencilAttachment_->getCurrentLayoutType();
    barrier.newLayout           = ResourceLayout::ShaderReadOnly;
    cmdBuffer->resourceBarrier(barrier);
  }
}

}  // namespace rhi
}  // namespace gfx
}  // namespace game_engine