#include "gfx/rhi/backends/vulkan/render_pass_vk.h"

#include "gfx/rhi/backends/vulkan/device_vk.h"
#include "gfx/rhi/backends/vulkan/rhi_enums_vk.h"
#include "utils/logger/global_logger.h"

namespace game_engine {
namespace gfx {
namespace rhi {

RenderPassVk::RenderPassVk(const RenderPassDesc& desc, DeviceVk* device)
    : m_device_(device)
    , m_hasDepthStencil_(desc.hasDepthStencil) {
  if (!initialize_(desc)) {
    GlobalLogger::Log(LogLevel::Error, "Failed to initialize render pass");
  }
}

RenderPassVk::~RenderPassVk() {
  if (m_renderPass_ != VK_NULL_HANDLE && m_device_) {
    vkDestroyRenderPass(m_device_->getDevice(), m_renderPass_, nullptr);
    m_renderPass_ = VK_NULL_HANDLE;
  }
}

bool RenderPassVk::initialize_(const RenderPassDesc& desc) {
  // Store load/store operations for shouldClear* methods
  m_colorAttachmentOps_.resize(desc.colorAttachments.size());
  for (size_t i = 0; i < desc.colorAttachments.size(); i++) {
    m_colorAttachmentOps_[i] = desc.colorAttachments[i].loadStoreOp;
  }

  if (desc.hasDepthStencil) {
    m_depthStencilAttachmentOp_ = desc.depthStencilAttachment.loadStoreOp;
  }

  // Create attachment descriptions
  std::vector<VkAttachmentDescription> attachmentDescs;
  attachmentDescs.reserve(desc.colorAttachments.size() + (desc.hasDepthStencil ? 1 : 0));

  // Color attachments
  for (const auto& colorAttachment : desc.colorAttachments) {
    VkAttachmentDescription attachmentDesc = {};
    attachmentDesc.format                  = g_getTextureFormatVk(colorAttachment.format);
    attachmentDesc.samples                 = static_cast<VkSampleCountFlagBits>(colorAttachment.samples);

    // Get load/store ops
    VkAttachmentLoadOp  loadOp;
    VkAttachmentStoreOp storeOp;
    g_getAttachmentLoadStoreOpVk(loadOp, storeOp, colorAttachment.loadStoreOp);

    attachmentDesc.loadOp         = loadOp;
    attachmentDesc.storeOp        = storeOp;
    attachmentDesc.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachmentDesc.initialLayout  = g_getImageLayoutVk(colorAttachment.initialLayout);
    attachmentDesc.finalLayout    = g_getImageLayoutVk(colorAttachment.finalLayout);

    attachmentDescs.push_back(attachmentDesc);
  }

  // Depth/stencil attachment (if any)
  VkAttachmentDescription depthAttachmentDesc = {};
  if (desc.hasDepthStencil) {
    depthAttachmentDesc.format  = g_getTextureFormatVk(desc.depthStencilAttachment.format);
    depthAttachmentDesc.samples = static_cast<VkSampleCountFlagBits>(desc.depthStencilAttachment.samples);

    // Get load/store ops
    VkAttachmentLoadOp  loadOp, stencilLoadOp;
    VkAttachmentStoreOp storeOp, stencilStoreOp;
    g_getAttachmentLoadStoreOpVk(loadOp, storeOp, desc.depthStencilAttachment.loadStoreOp);
    g_getAttachmentLoadStoreOpVk(stencilLoadOp, stencilStoreOp, desc.depthStencilAttachment.stencilLoadStoreOp);

    depthAttachmentDesc.loadOp         = loadOp;
    depthAttachmentDesc.storeOp        = storeOp;
    depthAttachmentDesc.stencilLoadOp  = stencilLoadOp;
    depthAttachmentDesc.stencilStoreOp = stencilStoreOp;
    depthAttachmentDesc.initialLayout  = g_getImageLayoutVk(desc.depthStencilAttachment.initialLayout);
    depthAttachmentDesc.finalLayout    = g_getImageLayoutVk(desc.depthStencilAttachment.finalLayout);

    attachmentDescs.push_back(depthAttachmentDesc);
  }

  // Color attachment references
  std::vector<VkAttachmentReference> colorAttachmentRefs;
  colorAttachmentRefs.reserve(desc.colorAttachments.size());

  for (size_t i = 0; i < desc.colorAttachments.size(); i++) {
    VkAttachmentReference attachmentRef = {};
    attachmentRef.attachment            = static_cast<uint32_t>(i);
    attachmentRef.layout                = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachmentRefs.push_back(attachmentRef);
  }

  // Depth attachment reference (if any)
  VkAttachmentReference depthAttachmentRef = {};
  if (desc.hasDepthStencil) {
    depthAttachmentRef.attachment = static_cast<uint32_t>(desc.colorAttachments.size());
    depthAttachmentRef.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
  }

  // Subpass description (just one subpass for now)
  VkSubpassDescription subpass = {};
  subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = static_cast<uint32_t>(colorAttachmentRefs.size());
  subpass.pColorAttachments    = colorAttachmentRefs.data();

  if (desc.hasDepthStencil) {
    subpass.pDepthStencilAttachment = &depthAttachmentRef;
  }

  // Create a simple dependency for the subpass
  VkSubpassDependency dependency = {};
  dependency.srcSubpass          = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass          = 0;
  dependency.srcStageMask        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
                          | (desc.hasDepthStencil ? VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT : 0);
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
                          | (desc.hasDepthStencil ? VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT : 0);
  dependency.srcAccessMask = 0;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
                           | (desc.hasDepthStencil ? VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT : 0);
  dependency.dependencyFlags = 0;

  // Create the render pass
  VkRenderPassCreateInfo renderPassInfo = {};
  renderPassInfo.sType                  = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount        = static_cast<uint32_t>(attachmentDescs.size());
  renderPassInfo.pAttachments           = attachmentDescs.data();
  renderPassInfo.subpassCount           = 1;
  renderPassInfo.pSubpasses             = &subpass;
  renderPassInfo.dependencyCount        = 1;
  renderPassInfo.pDependencies          = &dependency;

  VkResult result = vkCreateRenderPass(m_device_->getDevice(), &renderPassInfo, nullptr, &m_renderPass_);
  if (result != VK_SUCCESS) {
    GlobalLogger::Log(LogLevel::Error, "Failed to create render pass");
    return false;
  }

  return true;
}

bool RenderPassVk::shouldClearColor(uint32_t attachmentIndex) const {
  if (attachmentIndex >= m_colorAttachmentOps_.size()) {
    return false;
  }

  return m_colorAttachmentOps_[attachmentIndex] == AttachmentLoadStoreOp::ClearStore
      || m_colorAttachmentOps_[attachmentIndex] == AttachmentLoadStoreOp::ClearDontcare;
}

bool RenderPassVk::shouldClearDepthStencil() const {
  if (!m_hasDepthStencil_) {
    return false;
  }

  return m_depthStencilAttachmentOp_ == AttachmentLoadStoreOp::ClearStore
      || m_depthStencilAttachmentOp_ == AttachmentLoadStoreOp::ClearDontcare;
}

bool RenderPassVk::shouldClearStencil() const {
  // For simplicity, we're using the same load operation for both depth and stencil
  return shouldClearDepthStencil();
}

}  // namespace rhi
}  // namespace gfx
}  // namespace game_engine