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
  // ======= BEGIN: public constructors =======================================

  Attachment() = default;

  Attachment(const std::shared_ptr<RenderTarget>& rtPtr,
             EAttachmentLoadStoreOp               loadStoreOp
             = EAttachmentLoadStoreOp::CLEAR_STORE,
             EAttachmentLoadStoreOp stencilLoadStoreOp
             = EAttachmentLoadStoreOp::CLEAR_STORE,
             RtClearValue rtClearValue = RtClearValue(0.0f, 0.0f, 0.0f, 1.0f),
             EResourceLayout initialLayout = EResourceLayout::UNDEFINED,
             EResourceLayout finalLayout   = EResourceLayout::SHADER_READ_ONLY,
             bool            isResolveAttachment = false)
      : m_renderTarget_(rtPtr)
      , m_loadStoreOp_(loadStoreOp)
      , m_stencilLoadStoreOp_(stencilLoadStoreOp)
      , m_rtClearValue(rtClearValue)
      , m_initialLayout_(initialLayout)
      , m_finalLayout_(finalLayout)
      , m_ResolveAttachment(isResolveAttachment) {}

  // ======= END: public constructors   =======================================

  // ======= BEGIN: public getters ============================================

  size_t getHash() const {
    if (m_hash_) {
      return m_hash_;
    }

    m_hash_ = GETHASH_FROM_INSTANT_STRUCT(
        (m_renderTarget_ ? m_renderTarget_->getHash() : 0),
        m_loadStoreOp_,
        m_stencilLoadStoreOp_,
        m_rtClearValue,
        m_initialLayout_,
        m_finalLayout_);
    return m_hash_;
  }

  // ======= END: public getters   ============================================

  // ======= BEGIN: public misc methods =======================================

  bool isValid() const { return m_renderTarget_ != nullptr; }

  bool isDepthAttachment() const {
    assert(m_renderTarget_);
    return s_isDepthFormat(m_renderTarget_->m_info_.m_format_);
  }

  bool isResolveAttachment() const { return m_ResolveAttachment; }

  // ======= END: public misc methods   =======================================

  // ======= BEGIN: public misc fields ========================================

  std::shared_ptr<RenderTarget> m_renderTarget_;

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
  EAttachmentLoadStoreOp m_loadStoreOp_ = EAttachmentLoadStoreOp::CLEAR_STORE;
  EAttachmentLoadStoreOp m_stencilLoadStoreOp_
      = EAttachmentLoadStoreOp::CLEAR_STORE;

  RtClearValue m_rtClearValue = RtClearValue(0.0f, 0.0f, 0.0f, 1.0f);

  EResourceLayout m_initialLayout_ = EResourceLayout::UNDEFINED;
  EResourceLayout m_finalLayout_   = EResourceLayout::SHADER_READ_ONLY;

  // TODO: consider rename
  bool m_ResolveAttachment = false;

  mutable size_t m_hash_ = 0;

  // ======= END: public misc fields   ========================================
};

struct Subpass {
  // ======= BEGIN: public getters ============================================

  size_t getHash() const {
    size_t hash = 0;
    if (m_inputAttachments_.size() > 0) {
      hash = ::XXH64(m_inputAttachments_.data(),
                     m_inputAttachments_.size() * sizeof(int32_t),
                     hash);
    }
    if (m_outputColorAttachments_.size() > 0) {
      hash = ::XXH64(m_outputColorAttachments_.data(),
                     m_outputColorAttachments_.size() * sizeof(int32_t),
                     hash);
    }
    if (m_outputDepthAttachment_) {
      hash = ::XXH64(&m_outputDepthAttachment_.value(), sizeof(int32_t), hash);
    }
    if (m_outputResolveAttachment_) {
      hash
          = ::XXH64(&m_outputResolveAttachment_.value(), sizeof(int32_t), hash);
    }
    hash = ::XXH64(&m_sourceSubpassIndex_, sizeof(int32_t), hash);
    hash = ::XXH64(&m_destSubpassIndex_, sizeof(int32_t), hash);
    hash = ::XXH64(
        &m_attachmentProducePipelineBit_, sizeof(EPipelineStageMask), hash);
    hash = ::XXH64(
        &m_attachmentConsumePipelineBit_, sizeof(EPipelineStageMask), hash);
    return hash;
  }

  // ======= END: public getters   ============================================

  // ======= BEGIN: public misc methods =======================================

  bool isSubpassForExecuteInOrder() const {
    if ((m_sourceSubpassIndex_ == -1) && (m_destSubpassIndex_ == -1)) {
      return true;
    }

    if (m_sourceSubpassIndex_ != -1 && m_destSubpassIndex_ != -1) {
      return false;
    }

    // Subpass indices have to be either -1 for all or not for all.
    // This is an error condition and should be handled appropriately.
    assert(0);
    return false;
  }

  void initialize(int32_t            sourceSubpassIndex,
                  int32_t            destSubpassIndex,
                  EPipelineStageMask attachmentProducePipelineBit,
                  EPipelineStageMask attachmentConsumePipelineBit) {
    m_sourceSubpassIndex_           = sourceSubpassIndex;
    m_destSubpassIndex_             = destSubpassIndex;
    m_attachmentProducePipelineBit_ = attachmentProducePipelineBit;
    m_attachmentConsumePipelineBit_ = attachmentConsumePipelineBit;
  }

  // ======= END: public misc methods   =======================================

  // ======= BEGIN: public misc fields ========================================

  std::vector<int32_t> m_inputAttachments_;

  std::vector<int32_t>   m_outputColorAttachments_;
  std::optional<int32_t> m_outputDepthAttachment_;
  std::optional<int32_t> m_outputResolveAttachment_;

  // If both SourceSubpass and DstSubpass of all subpasses are -1, subpasses
  // will be executed in order
  int32_t m_sourceSubpassIndex_ = -1;
  int32_t m_destSubpassIndex_   = -1;

  // Default is the most strong pipeline stage mask
  EPipelineStageMask m_attachmentProducePipelineBit_ = EPipelineStageMask::
      COLOR_ATTACHMENT_OUTPUT_BIT;  // The pipeline which attachments would use
                                    // for this subpass
  EPipelineStageMask m_attachmentConsumePipelineBit_
      = EPipelineStageMask::FRAGMENT_SHADER_BIT;  // The pipeline which
                                                  // attachments would use for
                                                  // next subapss

  // ======= END: public misc fields   ========================================
};

struct RenderPassInfo {
  // ======= BEGIN: public getters ============================================

  size_t getHash() const {
    int64_t hash = 0;
    for (const auto& iter : m_attachments_) {
      hash = XXH64(iter.getHash(), hash);
    }
    for (const auto& iter : m_subpasses_) {
      hash = XXH64(iter.getHash(), hash);
    }
    return hash;
  }

  void reset() {
    m_attachments_.clear();
    m_subpasses_.clear();
  }

  // ======= END: public getters   ============================================

  // ======= BEGIN: public misc methods =======================================

  // If both SourceSubpass and DstSubpass of all subpasses are -1, subpasses
  // will be executed in order
  bool isSubpassForExecuteInOrder() const {
    assert(m_subpasses_.size());

    int32_t i = 0;
    bool    isSubpassForExecuteInOrder
        = m_subpasses_[i++].isSubpassForExecuteInOrder();
    for (; i < (int32_t)m_subpasses_.size(); ++i) {
      // All isSubpassForExecuteInOrder of subpasses must be same.
      assert(isSubpassForExecuteInOrder
             == m_subpasses_[i].isSubpassForExecuteInOrder());

      if (isSubpassForExecuteInOrder
          != m_subpasses_[i].isSubpassForExecuteInOrder()) {
        return false;
      }
    }
    return isSubpassForExecuteInOrder;
  }

  bool validate() const {
    for (const auto& iter : m_subpasses_) {
      for (const auto& inputIndex : iter.m_inputAttachments_) {
        assert(m_attachments_.size() > inputIndex);
        if (m_attachments_.size() > inputIndex) {
          return false;
        }
      }
      for (const auto& outputIndex : iter.m_outputColorAttachments_) {
        assert(m_attachments_.size() > outputIndex);

        if (m_attachments_.size() > outputIndex) {
          return false;
        }
      }
      if (iter.m_outputDepthAttachment_) {
        assert(m_attachments_.size() > iter.m_outputDepthAttachment_.value());

        if (m_attachments_.size() > iter.m_outputDepthAttachment_.value()) {
          return false;
        }
      }
      if (iter.m_outputResolveAttachment_) {
        assert(m_attachments_.size() > iter.m_outputResolveAttachment_.value());

        if (m_attachments_.size() > iter.m_outputResolveAttachment_.value()) {
          return false;
        }
      }
    }
    return true;
  }

  // ======= END: public misc methods   =======================================

  // ======= BEGIN: public misc fields ========================================

  std::vector<Attachment> m_attachments_;
  std::vector<Subpass>    m_subpasses_;

  // ======= END: public misc fields   ========================================
};

class RenderPass {
  public:
  // ======= BEGIN: public constructors =======================================

  RenderPass() = default;

  RenderPass(const RenderPassInfo&  renderPassInfo,
             const math::Vector2Di& offset,
             const math::Vector2Di& extent) {
    m_renderPassInfo_ = renderPassInfo;
    setRenderArea(offset, extent);
  }

  RenderPass(const std::vector<Attachment>& colorAttachments,
             const math::Vector2Di&         offset,
             const math::Vector2Di&         extent) {
    setAttachemnt(colorAttachments);
    setRenderArea(offset, extent);
  }

  RenderPass(const std::vector<Attachment>& colorAttachments,
             const Attachment&              depthAttachment,
             const math::Vector2Di&         offset,
             const math::Vector2Di&         extent) {
    setAttachemnt(colorAttachments, depthAttachment);
    setRenderArea(offset, extent);
  }

  RenderPass(const std::vector<Attachment>& colorAttachments,
             const Attachment&              depthAttachment,
             const Attachment&              colorResolveAttachment,
             const math::Vector2Di&         offset,
             const math::Vector2Di&         extent) {
    setAttachemnt(colorAttachments, depthAttachment, colorResolveAttachment);
    setRenderArea(offset, extent);
  }

  // ======= END: public constructors   =======================================

  // ======= BEGIN: public destructor =========================================

  virtual ~RenderPass() {}

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public getters ============================================

  virtual size_t getHash() const final {
    if (m_hash_) {
      return m_hash_;
    }

    m_hash_ = GETHASH_FROM_INSTANT_STRUCT(
        m_renderPassInfo_.getHash(), m_renderOffset_, m_renderExtent_);
    return m_hash_;
  }

  virtual void* getRenderPass() const { return nullptr; }

  virtual void* getFrameBuffer() const { return nullptr; }

  // ======= END: public getters   ============================================

  // ======= BEGIN: public setters ============================================

  void setAttachemnt(const std::vector<Attachment>& colorAttachments) {
    // Add output color attachment
    const int32_t kStartColorIndex
        = (int32_t)m_renderPassInfo_.m_attachments_.size();
    m_renderPassInfo_.m_attachments_.insert(
        m_renderPassInfo_.m_attachments_.end(),
        colorAttachments.begin(),
        colorAttachments.end());

    if (m_renderPassInfo_.m_subpasses_.empty()) {
      m_renderPassInfo_.m_subpasses_.resize(1);
    }

    for (int32_t i = 0; i < (int32_t)colorAttachments.size(); ++i) {
      m_renderPassInfo_.m_subpasses_[0].m_outputColorAttachments_.push_back(
          kStartColorIndex + i);
    }
  }

  void setAttachemnt(const std::vector<Attachment>& colorAttachments,
                     const Attachment&              depthAttachment) {
    // Add output color attachment
    const int32_t kStartColorIndex
        = (int32_t)m_renderPassInfo_.m_attachments_.size();
    m_renderPassInfo_.m_attachments_.insert(
        m_renderPassInfo_.m_attachments_.end(),
        colorAttachments.begin(),
        colorAttachments.end());

    if (m_renderPassInfo_.m_subpasses_.empty()) {
      m_renderPassInfo_.m_subpasses_.resize(1);
    }

    for (int32_t i = 0; i < (int32_t)colorAttachments.size(); ++i) {
      m_renderPassInfo_.m_subpasses_[0].m_outputColorAttachments_.push_back(
          kStartColorIndex + i);
    }

    // Add output depth attachment
    const int32_t kStartDepthIndex
        = (int32_t)m_renderPassInfo_.m_attachments_.size();
    m_renderPassInfo_.m_attachments_.push_back(depthAttachment);

    m_renderPassInfo_.m_subpasses_[0].m_outputDepthAttachment_
        = kStartDepthIndex;
  }

  // TODO: consider using  std::optional and std::nullopt
  void setAttachemnt(const std::vector<Attachment>& colorAttachments,
                     const Attachment&              depthAttachment,
                     const Attachment&              colorResolveAttachment) {
    // Add output color attachment
    const int32_t kStartColorIndex
        = (int32_t)m_renderPassInfo_.m_attachments_.size();
    m_renderPassInfo_.m_attachments_.insert(
        m_renderPassInfo_.m_attachments_.end(),
        colorAttachments.begin(),
        colorAttachments.end());

    if (m_renderPassInfo_.m_subpasses_.empty()) {
      m_renderPassInfo_.m_subpasses_.resize(1);
    }

    for (int32_t i = 0; i < (int32_t)colorAttachments.size(); ++i) {
      m_renderPassInfo_.m_subpasses_[0].m_outputColorAttachments_.push_back(
          kStartColorIndex + i);
    }

    // Add output depth attachment
    const int32_t kStartDepthIndex
        = (int32_t)m_renderPassInfo_.m_attachments_.size();
    m_renderPassInfo_.m_attachments_.push_back(depthAttachment);

    m_renderPassInfo_.m_subpasses_[0].m_outputDepthAttachment_
        = kStartDepthIndex;

    // Add output resolve attachment
    const int32_t kStartResolveIndex
        = (int32_t)m_renderPassInfo_.m_attachments_.size();
    m_renderPassInfo_.m_attachments_.push_back(colorResolveAttachment);

    m_renderPassInfo_.m_subpasses_[0].m_outputResolveAttachment_
        = kStartResolveIndex;
  }

  void setRenderArea(const math::Vector2Di& offset,
                     const math::Vector2Di& extent) {
    m_renderOffset_ = offset;
    m_renderExtent_ = extent;
  }

  // ======= END: public setters   ============================================

  // ======= BEGIN: public misc methods =======================================

  virtual bool beginRenderPass(
      const std::shared_ptr<CommandBuffer>& commandBuffer) {
    return false;
  }

  virtual void endRenderPass() {}

  // ======= END: public misc methods   =======================================

  // ======= BEGIN: public misc fields ========================================

  RenderPassInfo m_renderPassInfo_;

  // std::vector<Attachment> m_colorAttachments_;
  // Attachment m_depthAttachment_;
  // Attachment m_colorAttachmentResolve_;

  // TODO: consider using Dimension2Di
  math::Vector2Di m_renderOffset_;
  math::Vector2Di m_renderExtent_;
  mutable size_t  m_hash_ = 0;

  // ======= END: public misc fields   ========================================
};

}  // namespace game_engine

#endif  // GAME_ENGINE_RENDER_PASS_H
