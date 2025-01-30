#ifndef GAME_ENGINE_PIPELINE_STATE_INFO_H
#define GAME_ENGINE_PIPELINE_STATE_INFO_H

#include "gfx/rhi/buffer.h"
#include "gfx/rhi/instant_struct.h"
#include "gfx/rhi/render_pass.h"
#include "gfx/rhi/resource_container.h"
#include "gfx/rhi/rhi_type.h"
#include "gfx/rhi/shader.h"
#include "gfx/rhi/shader_bindable_resource.h"
#include "gfx/rhi/shader_binding_layout.h"

#include <math_library/dimension.h>
#include <math_library/vector.h>

#include <cassert>
#include <cstdint>
#include <memory>
#include <string>

namespace game_engine {

//////////////////////////////////////////////////////////////////////////
// Viewport
//////////////////////////////////////////////////////////////////////////
struct Viewport {
  // ======= BEGIN: public constructors =======================================

  Viewport() = default;

  Viewport(int32_t x,
           int32_t y,
           int32_t width,
           int32_t height,
           float   minDepth = 0.0f,
           float   maxDepth = 1.0f)
      : m_x_(static_cast<float>(x))
      , m_y_(static_cast<float>(y))
      , m_width_(static_cast<float>(width))
      , m_height_(static_cast<float>(height))
      , m_minDepth_(minDepth)
      , m_maxDepth_(maxDepth) {}

  Viewport(float x,
           float y,
           float width,
           float height,
           float minDepth = 0.0f,
           float maxDepth = 1.0f)
      : m_x_(x)
      , m_y_(y)
      , m_width_(width)
      , m_height_(height)
      , m_minDepth_(minDepth)
      , m_maxDepth_(maxDepth) {}

  // ======= END: public constructors   =======================================

  // ======= BEGIN: public getters ============================================

  size_t getHash() const {
    if (m_hash_) {
      return m_hash_;
    }

    m_hash_ = GETHASH_FROM_INSTANT_STRUCT(
        m_x_, m_y_, m_width_, m_height_, m_minDepth_, m_maxDepth_);
    return m_hash_;
  }

  // ======= END: public getters   ============================================

  // ======= BEGIN: public misc fields ========================================

  float m_x_        = 0.0f;
  float m_y_        = 0.0f;
  float m_width_    = 0.0f;
  float m_height_   = 0.0f;
  float m_minDepth_ = 0.0f;
  float m_maxDepth_ = 1.0f;

  mutable size_t m_hash_ = 0;

  // ======= END: public misc fields   ========================================
};

//////////////////////////////////////////////////////////////////////////
// Scissor
//////////////////////////////////////////////////////////////////////////
struct Scissor {
  // ======= BEGIN: public constructors =======================================

  Scissor() = default;

  Scissor(int32_t x, int32_t y, int32_t width, int32_t height)
      : m_offset_(x, y)
      , m_extent_(width, height) {}

  // ======= END: public constructors   =======================================

  // ======= BEGIN: public getters ============================================

  size_t getHash() const {
    if (m_hash_) {
      return m_hash_;
    }

    m_hash_ = GETHASH_FROM_INSTANT_STRUCT(m_offset_, m_extent_);
    return m_hash_;
  }

  // ======= END: public getters   ============================================

  // ======= BEGIN: public misc fields ========================================

  math::Point2Di     m_offset_;
  math::Dimension2Di m_extent_;

  mutable size_t m_hash_ = 0;

  // ======= END: public misc fields   ========================================
};

//////////////////////////////////////////////////////////////////////////
// SamplerStateInfo
//////////////////////////////////////////////////////////////////////////
struct SamplerStateInfo : public ShaderBindableResource {
  // ======= BEGIN: public constructors =======================================

  // ======= END: public constructors   =======================================

  // ======= BEGIN: public destructor =========================================

  virtual ~SamplerStateInfo() {}

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overloaded operators ===============================

  virtual void initialize() { getHash(); }

  virtual void* getHandle() const { return nullptr; }

  virtual size_t getHash() const {
    if (m_hash_) {
      return m_hash_;
    }

    m_hash_ = GETHASH_FROM_INSTANT_STRUCT(m_minification_,
                                          m_magnification_,
                                          m_addressU_,
                                          m_addressV_,
                                          m_addressW_,
                                          m_mipLODBias_,
                                          m_maxAnisotropy_,
                                          m_textureComparisonMode_,
                                          m_isEnableComparisonMode_,
                                          m_comparisonFunc_,
                                          m_borderColor_,
                                          m_minLOD_,
                                          m_maxLOD_);
    return m_hash_;
  }

  // ======= END: public overloaded operators   ===============================

  // ======= BEGIN: public misc methods =======================================

  std::string toString() const {
    std::string result;
    result += g_enumToString(m_minification_);
    result += ",";
    result += g_enumToString(m_magnification_);
    result += ",";
    result += g_enumToString(m_addressU_);
    result += ",";
    result += g_enumToString(m_addressV_);
    result += ",";
    result += g_enumToString(m_addressW_);
    result += std::to_string(m_mipLODBias_);
    result += ",";
    result += std::to_string(m_maxAnisotropy_);
    result += ",";
    result += g_enumToString(m_textureComparisonMode_);
    result += ",";
    result += std::to_string(m_isEnableComparisonMode_);
    result += ",";
    result += g_enumToString(m_comparisonFunc_);
    result += ",";
    result += std::to_string(m_maxAnisotropy_);
    result += ",";

    result += "(";
    result += std::to_string(m_borderColor_.x());
    result += ",";
    result += std::to_string(m_borderColor_.y());
    result += ",";
    result += std::to_string(m_borderColor_.z());
    result += ",";
    result += std::to_string(m_borderColor_.w());
    result += ")";
    result += ",";

    result += std::to_string(m_minLOD_);
    result += ",";
    result += std::to_string(m_minLOD_);
    result += ",";

    return result;
  }

  // ======= END: public misc methods   =======================================

  // ======= BEGIN: public misc fields ========================================

  mutable size_t m_hash_ = 0;

  ETextureFilter      m_minification_  = ETextureFilter::NEAREST;
  ETextureFilter      m_magnification_ = ETextureFilter::NEAREST;
  ETextureAddressMode m_addressU_      = ETextureAddressMode::CLAMP_TO_EDGE;
  ETextureAddressMode m_addressV_      = ETextureAddressMode::CLAMP_TO_EDGE;
  ETextureAddressMode m_addressW_      = ETextureAddressMode::CLAMP_TO_EDGE;
  float               m_mipLODBias_    = 0.0f;
  float m_maxAnisotropy_ = 1.0f;  // if you anisotropy filtering tuned on, set
                                  // this variable greater than 1.
  ETextureComparisonMode m_textureComparisonMode_
      = ETextureComparisonMode::NONE;
  bool            m_isEnableComparisonMode_ = false;
  ECompareOp      m_comparisonFunc_         = ECompareOp::LESS;
  math::Vector4Df m_borderColor_ = math::Vector4Df(0.0f, 0.0f, 0.0f, 1.0f);
  float           m_minLOD_      = -FLT_MAX;
  float           m_maxLOD_      = FLT_MAX;

  // ======= END: public misc fields   ========================================
};

//////////////////////////////////////////////////////////////////////////
// RasterizationStateInfo
//////////////////////////////////////////////////////////////////////////
struct RasterizationStateInfo {
  // ======= BEGIN: public destructor =========================================

  virtual ~RasterizationStateInfo() {}

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overloaded operators ===============================

  virtual void initialize() { getHash(); }

  virtual size_t getHash() const {
    if (m_hash_) {
      return m_hash_;
    }

    m_hash_ = GETHASH_FROM_INSTANT_STRUCT(m_polygonMode_,
                                          m_cullMode_,
                                          m_frontFace_,
                                          m_depthBiasEnable_,
                                          m_depthBiasConstantFactor_,
                                          m_depthBiasClamp_,
                                          m_depthBiasSlopeFactor_,
                                          m_lineWidth_,
                                          m_depthClampEnable_,
                                          m_rasterizerDiscardEnable_,
                                          m_sampleCount_,
                                          m_sampleShadingEnable_,
                                          m_minSampleShading_,
                                          m_alphaToCoverageEnable_,
                                          m_alphaToOneEnable_);
    return m_hash_;
  }

  // ======= END: public overloaded operators   ===============================

  // ======= BEGIN: public misc fields ========================================

  mutable size_t m_hash_ = 0;

  EPolygonMode m_polygonMode_             = EPolygonMode::FILL;
  ECullMode    m_cullMode_                = ECullMode::BACK;
  EFrontFace   m_frontFace_               = EFrontFace::CCW;
  bool         m_depthBiasEnable_         = false;
  float        m_depthBiasConstantFactor_ = 0.0f;
  float        m_depthBiasClamp_          = 0.0f;
  float        m_depthBiasSlopeFactor_    = 0.0f;
  float        m_lineWidth_               = 1.0f;
  bool         m_depthClampEnable_        = false;
  bool         m_rasterizerDiscardEnable_ = false;

  EMSAASamples m_sampleCount_         = EMSAASamples::COUNT_1;
  bool         m_sampleShadingEnable_ = true;  // Turn on sample shading (also
                                       // alleviates aliasing within textures)
  float        m_minSampleShading_      = 0.2f;
  bool         m_alphaToCoverageEnable_ = false;
  bool         m_alphaToOneEnable_      = false;

  // ======= END: public misc fields   ========================================
};

//////////////////////////////////////////////////////////////////////////
// StencilOpStateInfo
//////////////////////////////////////////////////////////////////////////
struct StencilOpStateInfo {
  // ======= BEGIN: public destructor =========================================

  virtual ~StencilOpStateInfo() {}

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overloaded operators ===============================

  virtual void initialize() { getHash(); }

  virtual size_t getHash() const {
    if (m_hash_) {
      return m_hash_;
    }

    m_hash_ = GETHASH_FROM_INSTANT_STRUCT(m_failOp_,
                                          m_passOp_,
                                          m_depthFailOp_,
                                          m_compareOp_,
                                          m_compareMask_,
                                          m_writeMask_,
                                          m_reference_);
    return m_hash_;
  }

  // ======= END: public overloaded operators   ===============================

  // ======= BEGIN: public misc fields ========================================

  mutable size_t m_hash_ = 0;

  EStencilOp m_failOp_      = EStencilOp::KEEP;
  EStencilOp m_passOp_      = EStencilOp::KEEP;
  EStencilOp m_depthFailOp_ = EStencilOp::KEEP;
  ECompareOp m_compareOp_   = ECompareOp::NEVER;
  uint32_t   m_compareMask_ = 0;
  uint32_t   m_writeMask_   = 0;
  uint32_t   m_reference_   = 0;

  // ======= END: public misc fields   ========================================
};

//////////////////////////////////////////////////////////////////////////
// DepthStencilStateInfo
//////////////////////////////////////////////////////////////////////////
struct DepthStencilStateInfo {
  // ======= BEGIN: public destructor =========================================

  virtual ~DepthStencilStateInfo() {}

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overloaded operators ===============================

  virtual void initialize() { getHash(); }

  virtual size_t getHash() const {
    if (m_hash_) {
      return m_hash_;
    }

    m_hash_ = GETHASH_FROM_INSTANT_STRUCT(m_depthTestEnable_,
                                          m_depthWriteEnable_,
                                          m_depthCompareOp_,
                                          m_depthBoundsTestEnable_,
                                          m_stencilTestEnable_,
                                          (m_front_ ? m_front_->getHash() : 0),
                                          (m_back_ ? m_back_->getHash() : 0),
                                          m_minDepthBounds_,
                                          m_maxDepthBounds_);
    return m_hash_;
  }

  // ======= END: public overloaded operators   ===============================

  // ======= BEGIN: public misc fields ========================================

  mutable size_t m_hash_ = 0;

  bool                m_depthTestEnable_       = false;
  bool                m_depthWriteEnable_      = false;
  ECompareOp          m_depthCompareOp_        = ECompareOp::LEQUAL;
  bool                m_depthBoundsTestEnable_ = false;
  bool                m_stencilTestEnable_     = false;
  StencilOpStateInfo* m_front_                 = nullptr;
  StencilOpStateInfo* m_back_                  = nullptr;
  float               m_minDepthBounds_        = 0.0f;
  float               m_maxDepthBounds_        = 1.0f;

  // VkPipelineDepthStencilStateCreateFlags    m_flags_;

  // ======= END: public misc fields   ========================================
};

//////////////////////////////////////////////////////////////////////////
// BlendingStateInfo
//////////////////////////////////////////////////////////////////////////
struct BlendingStateInfo {
  // ======= BEGIN: public destructor =========================================

  virtual ~BlendingStateInfo() {}

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overloaded operators ===============================

  virtual void initialize() { getHash(); }

  virtual size_t getHash() const {
    if (m_hash_) {
      return m_hash_;
    }

    m_hash_ = GETHASH_FROM_INSTANT_STRUCT(m_blendEnable_,
                                          m_src_,
                                          m_dest_,
                                          m_blendOp_,
                                          m_srcAlpha_,
                                          m_destAlpha_,
                                          m_alphaBlendOp_,
                                          m_colorWriteMask_);
    return m_hash_;
  }

  // ======= END: public overloaded operators   ===============================

  // ======= BEGIN: public misc fields ========================================

  mutable size_t m_hash_ = 0;

  bool         m_blendEnable_    = false;
  EBlendFactor m_src_            = EBlendFactor::SRC_COLOR;
  EBlendFactor m_dest_           = EBlendFactor::ONE_MINUS_SRC_ALPHA;
  EBlendOp     m_blendOp_        = EBlendOp::ADD;
  EBlendFactor m_srcAlpha_       = EBlendFactor::SRC_ALPHA;
  EBlendFactor m_destAlpha_      = EBlendFactor::ONE_MINUS_SRC_ALPHA;
  EBlendOp     m_alphaBlendOp_   = EBlendOp::ADD;
  EColorMask   m_colorWriteMask_ = EColorMask::NONE;

  // VkPipelineColorBlendStateCreateFlags       m_flags_;
  // VkBool32                                   m_logicOpEnable_;
  // VkLogicOp                                  m_logicOp_;
  // uint32_t                                   m_attachmentCount_;
  // const VkPipelineColorBlendAttachmentState* m_pAttachments_;
  // float                                      m_blendConstants_[4];

  // ======= END: public misc fields   ========================================
};

//////////////////////////////////////////////////////////////////////////
// PipelineStateFixedInfo
//////////////////////////////////////////////////////////////////////////
struct PipelineStateFixedInfo {
  // ======= BEGIN: public constructors =======================================

  PipelineStateFixedInfo() = default;

  PipelineStateFixedInfo(RasterizationStateInfo*      rasterizationState,
                         DepthStencilStateInfo*       depthStencilState,
                         BlendingStateInfo*           blendingState,
                         const std::vector<Viewport>& viewports,
                         const std::vector<Scissor>&  scissors,
                         bool                         isUseVRS)
      : m_rasterizationState_(rasterizationState)
      , m_depthStencilState_(depthStencilState)
      , m_blendingState_(blendingState)
      , m_viewports_(viewports)
      , m_scissors_(scissors)
      , m_isUseVRS_(isUseVRS) {
    createHash();
  }

  PipelineStateFixedInfo(RasterizationStateInfo* rasterizationState,
                         DepthStencilStateInfo*  depthStencilState,
                         BlendingStateInfo*      blendingState,
                         const Viewport&         viewport,
                         const Scissor&          scissor,
                         bool                    isUseVRS)
      : m_rasterizationState_(rasterizationState)
      , m_depthStencilState_(depthStencilState)
      , m_blendingState_(blendingState)
      , m_viewports_({viewport})
      , m_scissors_({scissor})
      , m_isUseVRS_(isUseVRS) {
    createHash();
  }

  PipelineStateFixedInfo(
      RasterizationStateInfo*                   rasterizationState,
      DepthStencilStateInfo*                    depthStencilState,
      BlendingStateInfo*                        blendingState,
      const std::vector<EPipelineDynamicState>& dynamicStates,
      bool                                      isUseVRS)
      : m_rasterizationState_(rasterizationState)
      , m_depthStencilState_(depthStencilState)
      , m_blendingState_(blendingState)
      , m_dynamicStates_(dynamicStates)
      , m_isUseVRS_(isUseVRS) {
    createHash();
  }

  // ======= END: public constructors   =======================================

  // ======= BEGIN: public misc methods =======================================

  size_t createHash() const {
    if (m_hash_) {
      return m_hash_;
    }

    m_hash_ = 0;
    for (int32_t i = 0; i < m_viewports_.size(); ++i) {
      m_hash_ ^= (m_viewports_[i].getHash() ^ (i + 1));
    }

    for (int32_t i = 0; i < m_scissors_.size(); ++i) {
      m_hash_ ^= (m_scissors_[i].getHash() ^ (i + 1));
    }

    if (m_dynamicStates_.size() > 0) {
      m_hash_ = ::XXH64(&m_dynamicStates_[0],
                        sizeof(EPipelineDynamicState) * m_dynamicStates_.size(),
                        m_hash_);
    }

    if (m_rasterizationState_) {
      m_hash_ ^= m_rasterizationState_->getHash();
    }
    if (m_depthStencilState_) {
      m_hash_ ^= m_depthStencilState_->getHash();
    }
    if (m_blendingState_) {
      m_hash_ ^= m_blendingState_->getHash();
    }

    m_hash_ ^= (uint64_t)m_isUseVRS_;

    return m_hash_;
  }

  // ======= END: public misc methods   =======================================

  // ======= BEGIN: public misc fields ========================================

  std::vector<Viewport>              m_viewports_;
  std::vector<Scissor>               m_scissors_;
  std::vector<EPipelineDynamicState> m_dynamicStates_;

  RasterizationStateInfo* m_rasterizationState_ = nullptr;
  DepthStencilStateInfo*  m_depthStencilState_  = nullptr;
  BlendingStateInfo*      m_blendingState_      = nullptr;
  bool                    m_isUseVRS_           = false;

  mutable size_t m_hash_ = 0;

  // ======= END: public misc fields   ========================================
};

struct PushConstantRange {
  // ======= BEGIN: public constructors =======================================

  PushConstantRange() = default;

  PushConstantRange(EShaderAccessStageFlag accessStageFlag,
                    int32_t                offset,
                    int32_t                size)
      : m_accessStageFlag_(accessStageFlag)
      , m_offset_(offset)
      , m_size_(size) {}

  // ======= END: public constructors   =======================================

  // ======= BEGIN: public misc fields ========================================

  EShaderAccessStageFlag m_accessStageFlag_
      = EShaderAccessStageFlag::ALL_GRAPHICS;
  int32_t m_offset_ = 0;
  int32_t m_size_   = 0;

  // ======= END: public misc fields   ========================================
};

struct PushConstant {
  // ======= BEGIN: public constructors =======================================

  // TODO: consider sorting these constructors

  PushConstant() = default;

  PushConstant(const PushConstant& pushConstant) {
    assert(pushConstant.m_usedSize_ < 256);

    m_usedSize_ = pushConstant.m_usedSize_;
    // TODO: consider use std::copy (not only there)
    memcpy(m_data_, pushConstant.m_data_, pushConstant.m_usedSize_);
    m_pushConstantRanges_ = pushConstant.m_pushConstantRanges_;
    m_hash_               = pushConstant.m_hash_;
  }

  PushConstant(const char*            data,
               int32_t                size,
               EShaderAccessStageFlag shaderAccessStageFlag) {
    assert(size < 256);

    m_usedSize_ = size;
    memcpy(m_data_, data, size);
    m_pushConstantRanges_.add(
        PushConstantRange(shaderAccessStageFlag, 0, size));
    getHash();
  }

  PushConstant(const char*              data,
               int32_t                  size,
               const PushConstantRange& pushConstantRange) {
    assert(size < 256);

    m_usedSize_ = size;
    memcpy(m_data_, data, size);
    m_pushConstantRanges_.add(pushConstantRange);
    getHash();
  }

  PushConstant(const char*                                 data,
               int32_t                                     size,
               const ResourceContainer<PushConstantRange>& pushConstantRanges)
      : m_pushConstantRanges_(pushConstantRanges) {
    assert(size < 256);

    m_usedSize_ = size;
    memcpy(m_data_, data, size);
    getHash();
  }

  template <typename T>
  PushConstant(const T& data, EShaderAccessStageFlag shaderAccessStageFlag) {
    set(data, PushConstantRange(shaderAccessStageFlag, 0, sizeof(T)));
  }

  template <typename T>
  PushConstant(const T& data, const PushConstantRange& pushConstantRange) {
    set(data, pushConstantRange);
  }

  template <typename T>
  PushConstant(const T&                                    data,
               const ResourceContainer<PushConstantRange>& pushConstantRanges) {
    set(data, pushConstantRanges);
  }

  // ======= END: public constructors   =======================================

  // ======= BEGIN: public destructor =========================================

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public getters ============================================

  template <typename T>
  T& get() const {
    return *(T*)&m_data_[0];
  }

  size_t getHash() const {
    if (m_hash_) {
      return m_hash_;
    }

    m_hash_ = 0;
    if (m_pushConstantRanges_.m_numOfData_ > 0) {
      // TODO: be careful with sizeof(PushConstantRange) when added
      // incapsulation
      m_hash_ = ::XXH64(
          &m_pushConstantRanges_[0],
          sizeof(PushConstantRange) * m_pushConstantRanges_.m_numOfData_,
          m_hash_);
    }

    return m_hash_;
  }

  const void* getConstantData() const { return (void*)&m_data_[0]; }

  int32_t getSize() const { return m_usedSize_; }

  const ResourceContainer<PushConstantRange>* getPushConstantRanges() const {
    return &m_pushConstantRanges_;
  }

  // ======= END: public getters   ============================================

  // ======= BEGIN: public setters ============================================

  template <typename T>
  void set(const T& data, const PushConstantRange& pushConstantRange) {
    assert(sizeof(T) < 256);

    m_usedSize_ = sizeof(T);
    memcpy(m_data_, &data, sizeof(T));
    m_pushConstantRanges_.add(pushConstantRange);
    getHash();
  }

  template <typename T>
  void set(const T&                                    data,
           const ResourceContainer<PushConstantRange>& pushConstantRanges) {
    assert(sizeof(T) < 256);

    m_usedSize_ = sizeof(T);
    memcpy(m_data_, &data, sizeof(T));
    m_pushConstantRanges_ = pushConstantRanges;
    getHash();
  }

  // ======= END: public setters   ============================================

  // ======= BEGIN: public misc methods =======================================

  bool isValid() const { return m_usedSize_ > 0; }

  // ======= END: public misc methods   =======================================

  // ======= BEGIN: public overloaded operators ===============================

  PushConstant& operator=(const PushConstant& pushConstant) {
    assert(pushConstant.m_usedSize_ < 256);

    m_usedSize_ = pushConstant.m_usedSize_;
    memcpy(m_data_, pushConstant.m_data_, pushConstant.m_usedSize_);
    m_pushConstantRanges_ = pushConstant.m_pushConstantRanges_;
    m_hash_               = pushConstant.m_hash_;
    return *this;
  }

  // ======= END: public overloaded operators   ===============================

  // ======= BEGIN: public misc fields ========================================

  mutable size_t                       m_hash_ = 0;
  ResourceContainer<PushConstantRange> m_pushConstantRanges_;
  uint8_t                              m_data_[256];
  int32_t                              m_usedSize_ = 0;

  // ======= END: public misc fields   ========================================
};

// struct Shader;
// struct GraphicsPipelineShader;
// struct VertexBuffer;
// struct ShaderBindingLayout;
// class  RenderPass;
// struct RenderFrameContext;

//////////////////////////////////////////////////////////////////////////
// PipelineStateInfo
//////////////////////////////////////////////////////////////////////////
struct PipelineStateInfo {
  // ======= BEGIN: public nested types =======================================

  enum class EPipelineType : uint8_t {
    Graphics = 0,
    Compute,
    RayTracing
  };

  // ======= END: public nested types   =======================================

  // ======= BEGIN: public constructors =======================================

  PipelineStateInfo() = default;

  PipelineStateInfo(const PipelineStateFixedInfo*   pipelineStateFixed,
                    const GraphicsPipelineShader    shader,
                    const VertexBufferArray&        vertexBufferArray,
                    const RenderPass*               renderPass,
                    const ShaderBindingLayoutArray& shaderBindingLayoutArray,
                    const PushConstant*             pushConstant = nullptr,
                    int32_t                         subpassIndex = 0)
      : kPipelineStateFixed(pipelineStateFixed)
      , kGraphicsShader(shader)
      , m_vertexBufferArray_(vertexBufferArray)
      , kRenderPass(renderPass)
      , m_shaderBindingLayoutArray_(shaderBindingLayoutArray)
      , kPushConstant(pushConstant)
      , m_subpassIndex_(subpassIndex) {
    m_pipelineType_ = EPipelineType::Graphics;
  }

  PipelineStateInfo(std::shared_ptr<Shader>         computeShader,
                    const ShaderBindingLayoutArray& shaderBindingLayoutArray,
                    const PushConstant*             pushConstant = nullptr,
                    int32_t                         subpassIndex = 0)
      : computeShader(std::move(computeShader))
      , m_shaderBindingLayoutArray_(shaderBindingLayoutArray)
      , kPushConstant(pushConstant)
      , m_subpassIndex_(subpassIndex) {
    m_pipelineType_ = EPipelineType::Compute;
  }

  // PipelineStateInfo(
  //     const std::vector<RaytracingPipelineShader>& shader,
  //     const RaytracingPipelineData&                raytracingPipelineData,
  //     const ShaderBindingLayoutArray& shaderBindingLayoutArray, const
  //     PushConstant*                          pushConstant = nullptr,
  //     int32_t                                       subpassIndex = 0)
  //     : m_raytracingShaders_(shader)
  //     , m_raytracingPipelineData_(raytracingPipelineData)
  //     , m_shaderBindingLayoutArray_(shaderBindingLayoutArray)
  //     , kPushConstant(pushConstant)
  //     , SubpassIndex(subpassIndex) {
  //   PipelineType = EPipelineType::RayTracing;
  // }

  PipelineStateInfo(const PipelineStateInfo& pipelineState)
      : kPipelineStateFixed(pipelineState.kPipelineStateFixed)
      , kGraphicsShader(pipelineState.kGraphicsShader)
      , computeShader(pipelineState.computeShader)
      //, m_raytracingShaders_(pipelineState.m_raytracingShaders_)
      , m_pipelineType_(pipelineState.m_pipelineType_)
      , m_vertexBufferArray_(pipelineState.m_vertexBufferArray_)
      , kRenderPass(pipelineState.kRenderPass)
      , m_shaderBindingLayoutArray_(pipelineState.m_shaderBindingLayoutArray_)
      , kPushConstant(pipelineState.kPushConstant)
      , m_hash_(pipelineState.m_hash_)
      , m_subpassIndex_(pipelineState.m_subpassIndex_)
  //, m_raytracingPipelineData_(pipelineState.m_raytracingPipelineData_)
  {}

  PipelineStateInfo(PipelineStateInfo&& pipelineState) noexcept
      : kPipelineStateFixed(pipelineState.kPipelineStateFixed)
      , kGraphicsShader(pipelineState.kGraphicsShader)
      , computeShader(pipelineState.computeShader)
      //, m_raytracingShaders_(pipelineState.m_raytracingShaders_)
      , m_pipelineType_(pipelineState.m_pipelineType_)
      , m_vertexBufferArray_(pipelineState.m_vertexBufferArray_)
      , kRenderPass(pipelineState.kRenderPass)
      , m_shaderBindingLayoutArray_(pipelineState.m_shaderBindingLayoutArray_)
      , kPushConstant(pipelineState.kPushConstant)
      , m_hash_(pipelineState.m_hash_)
      , m_subpassIndex_(pipelineState.m_subpassIndex_)
  //, m_raytracingPipelineData_(pipelineState.m_raytracingPipelineData_)
  {}

  // ======= END: public constructors   =======================================

  // ======= BEGIN: public destructor =========================================

  virtual ~PipelineStateInfo() {}

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overridden methods =================================

  virtual void initialize() { getHash(); }

  virtual void* getHandle() const { return nullptr; }

  virtual void* getPipelineLayoutHandle() const { return nullptr; }

  virtual void* createGraphicsPipelineState() { return nullptr; }

  virtual void* createComputePipelineState() { return nullptr; }

  // TODO: implement in the future
  // virtual void* createRaytracingPipelineState() { return nullptr; }

  virtual void bind(const std::shared_ptr<CommandBuffer>& commandBuffer) const {
  }

  // ======= END: public overridden methods   =================================

  // ======= BEGIN: public getters ============================================

  size_t getHash() const;

  // ======= END: public getters   ============================================

  // ======= BEGIN: public misc fields ========================================

  mutable size_t m_hash_ = 0;

  EPipelineType                 m_pipelineType_ = EPipelineType::Graphics;
  const GraphicsPipelineShader  kGraphicsShader;
  std::shared_ptr<Shader>       computeShader = nullptr;
  // std::vector<RaytracingPipelineShader> m_raytracingShaders_;
  // RaytracingPipelineData                m_raytracingPipelineData_;
  const RenderPass*             kRenderPass = nullptr;
  VertexBufferArray             m_vertexBufferArray_;
  ShaderBindingLayoutArray      m_shaderBindingLayoutArray_;
  const PushConstant*           kPushConstant;
  const PipelineStateFixedInfo* kPipelineStateFixed = nullptr;
  int32_t                       m_subpassIndex_     = 0;

  // ======= END: public misc fields   ========================================
};

}  // namespace game_engine

#endif  // GAME_ENGINE_PIPELINE_STATE_INFO_H
