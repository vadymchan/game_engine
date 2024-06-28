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

class RenderPassVk : public jRenderPass {
  public:
  using jRenderPass::jRenderPass;

  virtual ~RenderPassVk() { Release(); }

  void Release();

  // bool Initialize(const RenderPassInfoVk& renderPassInfo) {
  //   RenderPassInfo = renderPassInfo;
  //   if (!CreateRenderPass()) {
  //     return false;
  //   }
  //   return CreateFrameBuffer();
  // }

  void Initialize() { CreateRenderPass(); }

  // TODO: add subpassContents param
  bool BeginRenderPass(const jCommandBuffer* commandBuffer
                       /*, VkSubpassContents      subpassContents
                       = VK_SUBPASS_CONTENTS_INLINE*/) override;

  void EndRenderPass() override;

  virtual void* GetRenderPass() const override { return RenderPass; }

  virtual void* GetFrameBuffer() const override { return FrameBuffer; }

  bool CreateRenderPass();

  // TODO: make private

  void SetFinalLayoutToAttachment(const jAttachment& attachment) const;

  const jCommandBuffer* CommandBuffer = nullptr;

  VkRenderPassBeginInfo     RenderPassBeginInfo{};
  // TODO: consider remove it (already have in attachment)
  std::vector<VkClearValue> ClearValues;
  VkRenderPass              RenderPass  = VK_NULL_HANDLE;
  VkFramebuffer             FrameBuffer = VK_NULL_HANDLE;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_RENDER_PASS_VK_H