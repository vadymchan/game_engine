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

  virtual ~RenderPassVk() { Release(); }

  void Release();

  // bool Initialize(const RenderPassInfoVk& renderPassInfo) {
  //   m_renderPassInfo = renderPassInfo;
  //   if (!CreateRenderPass()) {
  //     return false;
  //   }
  //   return CreateFrameBuffer();
  // }

  void Initialize() { CreateRenderPass(); }

  // TODO: add subpassContents param
  bool BeginRenderPass(const CommandBuffer* commandBuffer
                       /*, VkSubpassContents      subpassContents
                       = VK_SUBPASS_CONTENTS_INLINE*/) override;

  void EndRenderPass() override;

  virtual void* GetRenderPass() const override { return m_renderPass; }

  virtual void* GetFrameBuffer() const override { return m_frameBuffer; }

  bool CreateRenderPass();

  // TODO: make private

  void SetFinalLayoutToAttachment(const Attachment& attachment) const;

  const CommandBuffer* m_CommandBuffer = nullptr;

  VkRenderPassBeginInfo     RenderPassBeginInfo{};
  // TODO: consider remove it (already have in attachment)
  std::vector<VkClearValue> ClearValues;
  VkRenderPass              m_renderPass  = VK_NULL_HANDLE;
  VkFramebuffer             m_frameBuffer = VK_NULL_HANDLE;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_RENDER_PASS_VK_H