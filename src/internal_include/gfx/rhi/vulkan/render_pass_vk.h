#ifndef GAME_ENGINE_RENDER_PASS_VK_H
#define GAME_ENGINE_RENDER_PASS_VK_H

#include "gfx/rhi/vulkan/command_buffer_vk.h"
#include "gfx/rhi/vulkan/render_target_vk.h"
#include "utils/third_party/xxhash_util.h"

#include <math_library/vector.h>

#include <memory>
#include <optional>
#include <vector>

namespace game_engine {

struct AttachmentVk {
  AttachmentVk() = default;

  AttachmentVk(const std::shared_ptr<RenderTargetVk>& InRTPtr,
               EAttachmentLoadStoreOp                 InLoadStoreOp
               = EAttachmentLoadStoreOp::CLEAR_STORE,
               EAttachmentLoadStoreOp InStencilLoadStoreOp
               = EAttachmentLoadStoreOp::CLEAR_STORE,
               RTClearValueVk InClearValue
               = RTClearValueVk(0.0f, 0.0f, 0.0f, 1.0f),
               EResourceLayout InInitialLayout = EResourceLayout::UNDEFINED,
               EResourceLayout InFinalLayout
               = EResourceLayout::SHADER_READ_ONLY,
               bool InIsResolveAttachment = false)
      : RenderTargetPtr(InRTPtr)
      , LoadStoreOp(InLoadStoreOp)
      , StencilLoadStoreOp(InStencilLoadStoreOp)
      , RTClearValue(InClearValue)
      , InitialLayout(InInitialLayout)
      , FinalLayout(InFinalLayout)
      , IsResolveAttachment(InIsResolveAttachment) {}

  bool IsValid() const { return RenderTargetPtr != nullptr; }

  bool IsDepthAttachment() const {
    assert(RenderTargetPtr);
    return IsDepthFormat(RenderTargetPtr->Info.Format);
  }

  size_t GetHash() const {
    if (Hash) {
      return Hash;
    }

     Hash = GETHASH_FROM_INSTANT_STRUCT(
         (RenderTargetPtr ? RenderTargetPtr->GetHash() : 0),
         LoadStoreOp,
         StencilLoadStoreOp,
         RTClearValue,
         InitialLayout,
         FinalLayout);
    return Hash;
  }

  /*static bool IsDepthFormat(VkFormat format) {
    switch (format) {
      case VK_FORMAT_D16_UNORM:
      case VK_FORMAT_D16_UNORM_S8_UINT:
      case VK_FORMAT_X8_D24_UNORM_PACK32:
      case VK_FORMAT_D24_UNORM_S8_UINT:
      case VK_FORMAT_D32_SFLOAT:
      case VK_FORMAT_D32_SFLOAT_S8_UINT:
        return true;
      default:
        return false;
    }
  }*/

  std::shared_ptr<RenderTargetVk> RenderTargetPtr;

  EAttachmentLoadStoreOp LoadStoreOp = EAttachmentLoadStoreOp::CLEAR_STORE;
  EAttachmentLoadStoreOp StencilLoadStoreOp
      = EAttachmentLoadStoreOp::CLEAR_STORE;

  RTClearValueVk RTClearValue;

  EResourceLayout InitialLayout;
  EResourceLayout FinalLayout;

  bool           IsResolveAttachment;
  mutable size_t Hash = 0;
};

struct SubpassVk {
  void Initialize(int32_t            InSourceSubpassIndex,
                  int32_t            InDestSubpassIndex,
                  EPipelineStageMask InAttachmentProducePipelineBit,
                  EPipelineStageMask InAttachmentConsumePipelineBit);

  bool IsSubpassForExecuteInOrder() const;

  size_t GetHash() const {
    size_t Hash = 0;
    if (InputAttachments.size() > 0) {
      Hash = ::XXH64(InputAttachments.data(),
                     InputAttachments.size() * sizeof(int32_t),
                     Hash);
    }
    if (OutputColorAttachments.size() > 0) {
      Hash = ::XXH64(OutputColorAttachments.data(),
                     OutputColorAttachments.size() * sizeof(int32_t),
                     Hash);
    }
    if (OutputDepthAttachment) {
      Hash = ::XXH64(&OutputDepthAttachment.value(), sizeof(int32_t), Hash);
    }
    if (OutputResolveAttachment) {
      Hash = ::XXH64(&OutputResolveAttachment.value(), sizeof(int32_t), Hash);
    }
    Hash = ::XXH64(&SourceSubpassIndex, sizeof(int32_t), Hash);
    Hash = ::XXH64(&DestSubpassIndex, sizeof(int32_t), Hash);
    Hash = ::XXH64(
        &AttachmentProducePipelineBit, sizeof(EPipelineStageMask), Hash);
    Hash = ::XXH64(
        &AttachmentConsumePipelineBit, sizeof(EPipelineStageMask), Hash);
    return Hash;
  }

  std::vector<int32_t> InputAttachments;

  std::vector<int32_t>   OutputColorAttachments;
  std::optional<int32_t> OutputDepthAttachment;
  std::optional<int32_t> OutputResolveAttachment;

  // If both SourceSubpass and DstSubpass of all subpasses are -1, subpasses
  // will be executed in order
  int32_t SourceSubpassIndex = -1;
  int32_t DestSubpassIndex   = -1;

  // Default is the most strong pipeline stage mask
  EPipelineStageMask AttachmentProducePipelineBit = EPipelineStageMask::
      COLOR_ATTACHMENT_OUTPUT_BIT;  // The pipeline which attachments would use
                                    // for this subpass
  EPipelineStageMask AttachmentConsumePipelineBit
      = EPipelineStageMask::FRAGMENT_SHADER_BIT;  // The pipeline which
                                                  // attachments would use for
                                                  // next subapss
};

struct RenderPassInfoVk {
  std::vector<AttachmentVk> Attachments;
  AttachmentVk              ResolveAttachment;
  std::vector<SubpassVk>    Subpasses;

  void Reset() {
    Attachments.clear();
    Subpasses.clear();
  }

  // If both SourceSubpass and DstSubpass of all subpasses are -1, subpasses
  // will be executed in order
  bool IsSubpassForExecuteInOrder() const;

  size_t GetHash() const {
    int64_t Hash = 0;
    for (const auto& iter : Attachments) {
      Hash = XXH64(iter.GetHash(), Hash);
    }
    for (const auto& iter : Subpasses) {
      Hash = XXH64(iter.GetHash(), Hash);
    }
    return Hash;
  }

  bool Validate() const;
};

class RenderPassVk {
  public:
  RenderPassVk(const RenderPassInfoVk& renderPassInfo,
               const math::Vector2Di&  offset,
               const math::Vector2Di&  extent)
      : RenderPassInfo(renderPassInfo)
      , RenderOffset(offset)
      , RenderExtent(extent) {}

  RenderPassVk(const std::vector<AttachmentVk>& colorAttachments,
               const math::Vector2Di&           offset,
               const math::Vector2Di&           extent)
      : RenderOffset(offset)
      , RenderExtent(extent) {
    SetAttachment(colorAttachments);
  }

  RenderPassVk(const std::vector<AttachmentVk>& colorAttachments,
               const AttachmentVk&              depthAttachment,
               const math::Vector2Di&           offset,
               const math::Vector2Di&           extent)
      : DepthAttachment(depthAttachment)
      , RenderOffset(offset)
      , RenderExtent(extent) {
    SetAttachment(colorAttachments, depthAttachment);
  }

  RenderPassVk(const std::vector<AttachmentVk>& colorAttachments,
               const AttachmentVk&              depthAttachment,
               const AttachmentVk&              colorResolveAttachment,
               const math::Vector2Di&           offset,
               const math::Vector2Di&           extent)
      : DepthAttachment(depthAttachment)
      //, ColorAttachmentResolve(colorResolveAttachment)
      , RenderOffset(offset)
      , RenderExtent(extent) {
    SetAttachment(colorAttachments, depthAttachment, colorResolveAttachment);
  }

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

  void SetAttachment(const std::vector<AttachmentVk>&   colorAttachments,
                     const std::optional<AttachmentVk>& depthAttachment
                     = std::nullopt,
                     const std::optional<AttachmentVk>& colorResolveAttachment
                     = std::nullopt);

  void SetRenderArea(const math::Vector2Di& offset,
                     const math::Vector2Di& extent);

  void SetFinalLayoutToAttachment(const AttachmentVk& attachment) const;

  bool BeginRenderPass(const CommandBufferVk* commandBuffer,
                       VkSubpassContents      subpassContents
                       = VK_SUBPASS_CONTENTS_INLINE);

  void EndRenderPass();

  VkRenderPass GetRenderPass() const { return RenderPass; }

  VkFramebuffer GetFrameBuffer() const { return FrameBuffer; }

  size_t GetHash() const {
    if (Hash) {
      return Hash;
    }

     Hash = GETHASH_FROM_INSTANT_STRUCT(
         RenderPassInfo.GetHash(), RenderOffset, RenderExtent);
    return Hash;
  }

  // TODO: make private
  bool CreateRenderPass();

  const CommandBufferVk* CommandBuffer = nullptr;

  RenderPassInfoVk          RenderPassInfo;
  VkRenderPass              RenderPass  = VK_NULL_HANDLE;
  VkFramebuffer             FrameBuffer = VK_NULL_HANDLE;
  VkRenderPassBeginInfo     RenderPassBeginInfo{};
  // TODO: consider remove it (already have in attachment)
  std::vector<VkClearValue> ClearValues;

  std::vector<AttachmentVk> ColorAttachments;
  AttachmentVk              DepthAttachment;
  // resolve multisampling (not needed for now)
  // AttachmentVk              ColorAttachmentResolve;

  // TODO: consider using Dimention2Di
  math::Vector2Di RenderOffset;
  math::Vector2Di RenderExtent;

  mutable size_t Hash = 0;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_RENDER_PASS_VK_H