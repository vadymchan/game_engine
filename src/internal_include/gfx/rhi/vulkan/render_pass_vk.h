#ifndef GAME_ENGINE_RENDER_PASS_VK_H
#define GAME_ENGINE_RENDER_PASS_VK_H

#include "gfx/rhi/render_pass.h"
#include "gfx/rhi/vulkan/command_buffer_vk.h"
#include "utils/third_party/xxhash_util.h"

#include <math_library/vector.h>

#include <memory>
#include <optional>
#include <vector>

namespace game_engine {

class RenderPassVk : public RenderPass {
  public:
  // ======= BEGIN: public aliases ============================================

  using RenderPass::RenderPass;

  // ======= END: public aliases   ============================================

  // ======= BEGIN: public destructor =========================================

  virtual ~RenderPassVk() { release(); }

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overridden methods =================================

  // TODO: add subpassContents param
  bool beginRenderPass(const std::shared_ptr<CommandBuffer>& commandBuffer
                       /*, VkSubpassContents      subpassContents
                       = VK_SUBPASS_CONTENTS_INLINE*/) override;

  void endRenderPass() override;

  virtual void* getRenderPass() const override { return m_renderPass_; }

  virtual void* getFrameBuffer() const override { return m_frameBuffer_; }

  // ======= END: public overridden methods   =================================

  // ======= BEGIN: public setters ============================================

  // TODO: make private
  void setFinalLayoutToAttachment_(const Attachment& attachment) const;

  // ======= END: public setters   ============================================

  // ======= BEGIN: public misc methods =======================================

  void initialize() { createRenderPass(); }

  // bool initialize(const RenderPassInfoVk& renderPassInfo) {
  //   m_renderPassInfo_ = renderPassInfo;
  //   if (!createRenderPass()) {
  //     return false;
  //   }
  //   return createFrameBuffer();
  // }

  bool createRenderPass();

  void release();

  // ======= END: public misc methods   =======================================

  // ======= BEGIN: public misc fields ========================================

  std::shared_ptr<CommandBuffer> m_commandBuffer_ = nullptr;

  VkRenderPassBeginInfo     m_renderPassBeginInfo_{};
  // TODO: consider remove it (already have in attachment)
  std::vector<VkClearValue> m_clearValues_;
  VkRenderPass              m_renderPass_  = VK_NULL_HANDLE;
  VkFramebuffer             m_frameBuffer_ = VK_NULL_HANDLE;

  // ======= END: public misc fields   ========================================
};

}  // namespace game_engine

#endif  // GAME_ENGINE_RENDER_PASS_VK_H
