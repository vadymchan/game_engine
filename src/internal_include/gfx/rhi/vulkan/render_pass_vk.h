#ifndef GAME_ENGINE_RENDER_PASS_VK_H
#define GAME_ENGINE_RENDER_PASS_VK_H

#include "gfx/rhi/render_pass.h"
#include "gfx/rhi/vulkan/command_buffer_vk.h"
#include "gfx/rhi/vulkan/render_target_vk.h"
#include "utils/third_party/xxhash_util.h"

#include <math_library/vector.h>

#include <memory>
#include <optional>
#include <vector>

namespace game_engine {

class RenderPassVk : public RenderPass {
  public:
  using RenderPass::RenderPass;

  virtual ~RenderPassVk() { release(); }

  void release();

  // bool initialize(const RenderPassInfoVk& renderPassInfo) {
  //   m_renderPassInfo_ = renderPassInfo;
  //   if (!createRenderPass()) {
  //     return false;
  //   }
  //   return createFrameBuffer();
  // }

  void initialize() { createRenderPass(); }

  // TODO: add subpassContents param
  bool beginRenderPass(const CommandBuffer* commandBuffer
                       /*, VkSubpassContents      subpassContents
                       = VK_SUBPASS_CONTENTS_INLINE*/) override;

  void endRenderPass() override;

  virtual void* getRenderPass() const override { return m_renderPass_; }

  virtual void* getFrameBuffer() const override { return m_frameBuffer_; }

  bool createRenderPass();

  // TODO: make private

  void setFinalLayoutToAttachment_(const Attachment& attachment) const;

  const CommandBuffer* m_commandBuffer_ = nullptr;

  VkRenderPassBeginInfo     m_renderPassBeginInfo_{};
  // TODO: consider remove it (already have in attachment)
  std::vector<VkClearValue> m_clearValues_;
  VkRenderPass              m_renderPass_  = VK_NULL_HANDLE;
  VkFramebuffer             m_frameBuffer_ = VK_NULL_HANDLE;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_RENDER_PASS_VK_H