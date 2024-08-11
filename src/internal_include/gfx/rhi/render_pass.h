#ifndef GAME_ENGINE_RENDER_PASS_H
#define GAME_ENGINE_RENDER_PASS_H

#include "gfx/rhi/command_buffer_manager.h"
#include "gfx/rhi/render_target.h"
#include "gfx/rhi/rhi_type.h"

#include <cassert>
#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

namespace game_engine {

struct Attachment {
  Attachment() = default;

  Attachment(const std::shared_ptr<RenderTarget>& InRTPtr,
              EAttachmentLoadStoreOp                InLoadStoreOp
              = EAttachmentLoadStoreOp::CLEAR_STORE,
              EAttachmentLoadStoreOp InStencilLoadStoreOp
              = EAttachmentLoadStoreOp::CLEAR_STORE,
              RTClearValue rtClearValue
              = RTClearValue(0.0f, 0.0f, 0.0f, 1.0f),
              EResourceLayout InInitialLayout = EResourceLayout::UNDEFINED,
              EResourceLayout InFinalLayout = EResourceLayout::SHADER_READ_ONLY,
              bool            InIsResolveAttachment = false)
      : RenderTargetPtr(InRTPtr)
      , LoadStoreOp(InLoadStoreOp)
      , StencilLoadStoreOp(InStencilLoadStoreOp)
      , m_rtClearValue(rtClearValue)
      , InitialLayout(InInitialLayout)
      , FinalLayout(InFinalLayout)
      , bResolveAttachment(InIsResolveAttachment) {}

  std::shared_ptr<RenderTarget> RenderTargetPtr;

  // The two options below determine what to do with the data in the attachment
  // before and after rendering.
  // 1). loadOp
  // - VK_ATTACHMENT_LOAD_OP_LOAD: Maintain the contents of the attachment
  // as is
  // - VK_ATTACHMENT_LOAD_OP_CLEAR: Set all values in the attachment to a
  // constant value.
  // - VK_ATTACHMENT_LOAD_OP_DONT_CARE: Do nothing with the contents of the
  // attachment. Undefined state.
  // 2). storeOp
  // - VK_ATTACHMENT_STORE_OP_STORE: The rendered contents are stored in
  // memory and can be read later.
  // - VK_ATTACHMENT_STORE_OP_DONT_CARE: After rendering, the contents of
  // the framebuffer are undefined.
  EAttachmentLoadStoreOp LoadStoreOp = EAttachmentLoadStoreOp::CLEAR_STORE;
  EAttachmentLoadStoreOp StencilLoadStoreOp
      = EAttachmentLoadStoreOp::CLEAR_STORE;

  RTClearValue m_rtClearValue = RTClearValue(0.0f, 0.0f, 0.0f, 1.0f);

  EResourceLayout InitialLayout = EResourceLayout::UNDEFINED;
  EResourceLayout FinalLayout   = EResourceLayout::SHADER_READ_ONLY;

  bool bResolveAttachment = false;

  bool IsValid() const { return RenderTargetPtr != nullptr; }

  size_t GetHash() const {
    if (Hash) {
      return Hash;
    }

    Hash = GETHASH_FROM_INSTANT_STRUCT(
        (RenderTargetPtr ? RenderTargetPtr->GetHash() : 0),
        LoadStoreOp,
        StencilLoadStoreOp,
        m_rtClearValue,
        InitialLayout,
        FinalLayout);
    return Hash;
  }

  bool IsDepthAttachment() const {
    assert(RenderTargetPtr);
    return IsDepthFormat(RenderTargetPtr->Info.Format);
  }

  bool IsResolveAttachment() const { return bResolveAttachment; }

  mutable size_t Hash = 0;
};

struct Subpass {
  void Initialize(int32_t            InSourceSubpassIndex,
                  int32_t            InDestSubpassIndex,
                  EPipelineStageMask InAttachmentProducePipelineBit,
                  EPipelineStageMask InAttachmentConsumePipelineBit) {
    SourceSubpassIndex           = InSourceSubpassIndex;
    DestSubpassIndex             = InDestSubpassIndex;
    AttachmentProducePipelineBit = InAttachmentProducePipelineBit;
    AttachmentConsumePipelineBit = InAttachmentConsumePipelineBit;
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

  bool IsSubpassForExecuteInOrder() const {
    if ((SourceSubpassIndex == -1) && (DestSubpassIndex == -1)) {
      return true;
    }

    if (SourceSubpassIndex != -1 && DestSubpassIndex != -1) {
      return false;
    }

    // Subpass indices have to be either -1 for all or not for all.
    // This is an error condition and should be handled appropriately.
    assert(0);
    return false;
  }

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
};

struct RenderPassInfo {
  std::vector<Attachment> Attachments;
  Attachment              ResolveAttachment;
  std::vector<Subpass>    Subpasses;

  void Reset() {
    Attachments.clear();
    Subpasses.clear();
  }

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

  // If both SourceSubpass and DstSubpass of all subpasses are -1, subpasses
  // will be executed in order
  bool IsSubpassForExecuteInOrder() const {
    assert(Subpasses.size());

    int32_t i = 0;
    bool    isSubpassForExecuteInOrder
        = Subpasses[i++].IsSubpassForExecuteInOrder();
    for (; i < (int32_t)Subpasses.size(); ++i) {
      // All isSubpassForExecuteInOrder of subpasses must be same.
      assert(isSubpassForExecuteInOrder
             == Subpasses[i].IsSubpassForExecuteInOrder());

      if (isSubpassForExecuteInOrder
          != Subpasses[i].IsSubpassForExecuteInOrder()) {
        return false;
      }
    }
    return isSubpassForExecuteInOrder;
  }

  bool Validate() const {
    for (const auto& iter : Subpasses) {
      for (const auto& inputIndex : iter.InputAttachments) {
        assert(Attachments.size() > inputIndex);
        if (Attachments.size() > inputIndex) {
          return false;
        }
      }
      for (const auto& outputIndex : iter.OutputColorAttachments) {
        assert(Attachments.size() > outputIndex);

        if (Attachments.size() > outputIndex) {
          return false;
        }
      }
      if (iter.OutputDepthAttachment) {
        assert(Attachments.size() > iter.OutputDepthAttachment.value());

        if (Attachments.size() > iter.OutputDepthAttachment.value()) {
          return false;
        }
      }
      if (iter.OutputResolveAttachment) {
        assert(Attachments.size() > iter.OutputResolveAttachment.value());

        if (Attachments.size() > iter.OutputResolveAttachment.value()) {
          return false;
        }
      }
    }
    return true;
  }
};

class RenderPass {
  public:
  virtual ~RenderPass() {}

  RenderPass() = default;

  RenderPass(const std::vector<Attachment>& colorAttachments,
              const math::Vector2Di&          offset,
              const math::Vector2Di&          extent) {
    SetAttachemnt(colorAttachments);
    SetRenderArea(offset, extent);
  }

  RenderPass(const std::vector<Attachment>& colorAttachments,
              const Attachment&              depthAttachment,
              const math::Vector2Di&          offset,
              const math::Vector2Di&          extent) {
    SetAttachemnt(colorAttachments, depthAttachment);
    SetRenderArea(offset, extent);
  }

  RenderPass(const std::vector<Attachment>& colorAttachments,
              const Attachment&              depthAttachment,
              const Attachment&              colorResolveAttachment,
              const math::Vector2Di&          offset,
              const math::Vector2Di&          extent) {
    SetAttachemnt(colorAttachments, depthAttachment, colorResolveAttachment);
    SetRenderArea(offset, extent);
  }

  void SetAttachemnt(const std::vector<Attachment>& colorAttachments) {
    // Add output color attachment
    const int32_t startColorIndex = (int32_t)m_renderPassInfo.Attachments.size();
    m_renderPassInfo.Attachments.insert(m_renderPassInfo.Attachments.end(),
                                      colorAttachments.begin(),
                                      colorAttachments.end());

    if (m_renderPassInfo.Subpasses.empty()) {
      m_renderPassInfo.Subpasses.resize(1);
    }

    for (int32_t i = 0; i < (int32_t)colorAttachments.size(); ++i) {
      m_renderPassInfo.Subpasses[0].OutputColorAttachments.push_back(
          startColorIndex + i);
    }
  }

  void SetAttachemnt(const std::vector<Attachment>& colorAttachments,
                     const Attachment&              depthAttachment) {
    // Add output color attachment
    const int32_t startColorIndex = (int32_t)m_renderPassInfo.Attachments.size();
    m_renderPassInfo.Attachments.insert(m_renderPassInfo.Attachments.end(),
                                      colorAttachments.begin(),
                                      colorAttachments.end());

    if (m_renderPassInfo.Subpasses.empty()) {
      m_renderPassInfo.Subpasses.resize(1);
    }

    for (int32_t i = 0; i < (int32_t)colorAttachments.size(); ++i) {
      m_renderPassInfo.Subpasses[0].OutputColorAttachments.push_back(
          startColorIndex + i);
    }

    // Add output depth attachment
    const int32_t startDepthIndex = (int32_t)m_renderPassInfo.Attachments.size();
    m_renderPassInfo.Attachments.push_back(depthAttachment);

    m_renderPassInfo.Subpasses[0].OutputDepthAttachment = startDepthIndex;
  }

  // TODO: consider using  std::optional and std::nullopt
  void SetAttachemnt(const std::vector<Attachment>& colorAttachments,
                     const Attachment&              depthAttachment,
                     const Attachment&              colorResolveAttachment) {
    // Add output color attachment
    const int32_t startColorIndex = (int32_t)m_renderPassInfo.Attachments.size();
    m_renderPassInfo.Attachments.insert(m_renderPassInfo.Attachments.end(),
                                      colorAttachments.begin(),
                                      colorAttachments.end());

    if (m_renderPassInfo.Subpasses.empty()) {
      m_renderPassInfo.Subpasses.resize(1);
    }

    for (int32_t i = 0; i < (int32_t)colorAttachments.size(); ++i) {
      m_renderPassInfo.Subpasses[0].OutputColorAttachments.push_back(
          startColorIndex + i);
    }

    // Add output depth attachment
    const int32_t startDepthIndex = (int32_t)m_renderPassInfo.Attachments.size();
    m_renderPassInfo.Attachments.push_back(depthAttachment);

    m_renderPassInfo.Subpasses[0].OutputDepthAttachment = startDepthIndex;

    // Add output resolve attachment
    const int32_t startResolveIndex
        = (int32_t)m_renderPassInfo.Attachments.size();
    m_renderPassInfo.Attachments.push_back(colorResolveAttachment);

    m_renderPassInfo.Subpasses[0].OutputResolveAttachment = startResolveIndex;
  }

  void SetRenderArea(const math::Vector2Di& offset,
                     const math::Vector2Di& extent) {
    RenderOffset = offset;
    RenderExtent = extent;
  }

  RenderPass(const RenderPassInfo& renderPassInfo,
              const math::Vector2Di& offset,
              const math::Vector2Di& extent) {
    m_renderPassInfo = renderPassInfo;
    SetRenderArea(offset, extent);
  }

  virtual bool BeginRenderPass(const CommandBuffer* commandBuffer) {
    return false;
  }

  virtual void EndRenderPass() {}

  virtual size_t GetHash() const final {
    if (Hash) {
      return Hash;
    }

    Hash = GETHASH_FROM_INSTANT_STRUCT(
        m_renderPassInfo.GetHash(), RenderOffset, RenderExtent);
    return Hash;
  }

  virtual void* GetRenderPass() const { return nullptr; }

  virtual void* GetFrameBuffer() const { return nullptr; }

  RenderPassInfo m_renderPassInfo;

  // std::vector<Attachment> ColorAttachments;
  // Attachment DepthAttachment;
  // Attachment ColorAttachmentResolve;

  // TODO: consider using Dimension2Di
  math::Vector2Di RenderOffset;
  math::Vector2Di RenderExtent;
  mutable size_t  Hash = 0;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_RENDER_PASS_H