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
  Viewport() = default;

  Viewport(int32_t x,
           int32_t y,
           int32_t width,
           int32_t height,
           float   minDepth = 0.0f,
           float   maxDepth = 1.0f)
      : X(static_cast<float>(x))
      , Y(static_cast<float>(y))
      , Width(static_cast<float>(width))
      , Height(static_cast<float>(height))
      , MinDepth(minDepth)
      , MaxDepth(maxDepth) {}

  Viewport(float x,
           float y,
           float width,
           float height,
           float minDepth = 0.0f,
           float maxDepth = 1.0f)
      : X(x)
      , Y(y)
      , Width(width)
      , Height(height)
      , MinDepth(minDepth)
      , MaxDepth(maxDepth) {}

  float X        = 0.0f;
  float Y        = 0.0f;
  float Width    = 0.0f;
  float Height   = 0.0f;
  float MinDepth = 0.0f;
  float MaxDepth = 1.0f;

  mutable size_t Hash = 0;

  size_t GetHash() const {
    if (Hash) {
      return Hash;
    }

    Hash = GETHASH_FROM_INSTANT_STRUCT(X, Y, Width, Height, MinDepth, MaxDepth);
    return Hash;
  }
};

//////////////////////////////////////////////////////////////////////////
// Scissor
//////////////////////////////////////////////////////////////////////////
struct Scissor {
  Scissor() = default;

  Scissor(int32_t x, int32_t y, int32_t width, int32_t height)
      : Offset(x, y)
      , Extent(width, height) {}

  math::Point2Di     Offset;
  math::Dimension2Di Extent;

  mutable size_t Hash = 0;

  size_t GetHash() const {
    if (Hash) {
      return Hash;
    }

    Hash = GETHASH_FROM_INSTANT_STRUCT(Offset, Extent);
    return Hash;
  }
};

//////////////////////////////////////////////////////////////////////////
// SamplerStateInfo
//////////////////////////////////////////////////////////////////////////
struct SamplerStateInfo : public ShaderBindableResource {
  virtual ~SamplerStateInfo() {}

  virtual void Initialize() { GetHash(); }

  virtual void* GetHandle() const { return nullptr; }

  virtual size_t GetHash() const {
    if (Hash) {
      return Hash;
    }

    Hash = GETHASH_FROM_INSTANT_STRUCT(Minification,
                                       Magnification,
                                       AddressU,
                                       AddressV,
                                       AddressW,
                                       MipLODBias,
                                       MaxAnisotropy,
                                       TextureComparisonMode,
                                       IsEnableComparisonMode,
                                       ComparisonFunc,
                                       BorderColor,
                                       MinLOD,
                                       MaxLOD);
    return Hash;
  }

  std::string ToString() const {
    std::string Result;
    Result += EnumToString(Minification);
    Result += ",";
    Result += EnumToString(Magnification);
    Result += ",";
    Result += EnumToString(AddressU);
    Result += ",";
    Result += EnumToString(AddressV);
    Result += ",";
    Result += EnumToString(AddressW);
    Result += std::to_string(MipLODBias);
    Result += ",";
    Result += std::to_string(MaxAnisotropy);
    Result += ",";
    Result += EnumToString(TextureComparisonMode);
    Result += ",";
    Result += std::to_string(IsEnableComparisonMode);
    Result += ",";
    Result += EnumToString(ComparisonFunc);
    Result += ",";
    Result += std::to_string(MaxAnisotropy);
    Result += ",";

    Result += "(";
    Result += std::to_string(BorderColor.x());
    Result += ",";
    Result += std::to_string(BorderColor.y());
    Result += ",";
    Result += std::to_string(BorderColor.z());
    Result += ",";
    Result += std::to_string(BorderColor.w());
    Result += ")";
    Result += ",";

    Result += std::to_string(MinLOD);
    Result += ",";
    Result += std::to_string(MinLOD);
    Result += ",";

    return Result;
  }

  mutable size_t Hash = 0;

  ETextureFilter      Minification  = ETextureFilter::NEAREST;
  ETextureFilter      Magnification = ETextureFilter::NEAREST;
  ETextureAddressMode AddressU      = ETextureAddressMode::CLAMP_TO_EDGE;
  ETextureAddressMode AddressV      = ETextureAddressMode::CLAMP_TO_EDGE;
  ETextureAddressMode AddressW      = ETextureAddressMode::CLAMP_TO_EDGE;
  float               MipLODBias    = 0.0f;
  float MaxAnisotropy = 1.0f;  // if you anisotropy filtering tuned on, set this
                               // variable greater than 1.
  ETextureComparisonMode TextureComparisonMode  = ETextureComparisonMode::NONE;
  bool                   IsEnableComparisonMode = false;
  ECompareOp             ComparisonFunc         = ECompareOp::LESS;
  math::Vector4Df        BorderColor = math::Vector4Df(0.0f, 0.0f, 0.0f, 1.0f);
  float                  MinLOD      = -FLT_MAX;
  float                  MaxLOD      = FLT_MAX;
};

//////////////////////////////////////////////////////////////////////////
// RasterizationStateInfo
//////////////////////////////////////////////////////////////////////////
struct RasterizationStateInfo {
  virtual ~RasterizationStateInfo() {}

  virtual void Initialize() { GetHash(); }

  virtual size_t GetHash() const {
    if (Hash) {
      return Hash;
    }

    Hash = GETHASH_FROM_INSTANT_STRUCT(PolygonMode,
                                       CullMode,
                                       FrontFace,
                                       DepthBiasEnable,
                                       DepthBiasConstantFactor,
                                       DepthBiasClamp,
                                       DepthBiasSlopeFactor,
                                       LineWidth,
                                       DepthClampEnable,
                                       RasterizerDiscardEnable,
                                       SampleCount,
                                       SampleShadingEnable,
                                       MinSampleShading,
                                       AlphaToCoverageEnable,
                                       AlphaToOneEnable);
    return Hash;
  }

  mutable size_t Hash = 0;

  EPolygonMode PolygonMode             = EPolygonMode::FILL;
  ECullMode    CullMode                = ECullMode::BACK;
  EFrontFace   FrontFace               = EFrontFace::CCW;
  bool         DepthBiasEnable         = false;
  float        DepthBiasConstantFactor = 0.0f;
  float        DepthBiasClamp          = 0.0f;
  float        DepthBiasSlopeFactor    = 0.0f;
  float        LineWidth               = 1.0f;
  bool         DepthClampEnable        = false;
  bool         RasterizerDiscardEnable = false;

  EMSAASamples SampleCount  = EMSAASamples::COUNT_1;
  bool  SampleShadingEnable = true;  // Turn on sample shading (also alleviates
                                     // aliasing within textures)
  float MinSampleShading      = 0.2f;
  bool  AlphaToCoverageEnable = false;
  bool  AlphaToOneEnable      = false;
};

//////////////////////////////////////////////////////////////////////////
// StencilOpStateInfo
//////////////////////////////////////////////////////////////////////////
struct StencilOpStateInfo {
  virtual ~StencilOpStateInfo() {}

  virtual void Initialize() { GetHash(); }

  virtual size_t GetHash() const {
    if (Hash) {
      return Hash;
    }

    Hash = GETHASH_FROM_INSTANT_STRUCT(FailOp,
                                       PassOp,
                                       DepthFailOp,
                                       CompareOp,
                                       CompareMask,
                                       WriteMask,
                                       Reference);
    return Hash;
  }

  mutable size_t Hash = 0;

  EStencilOp FailOp      = EStencilOp::KEEP;
  EStencilOp PassOp      = EStencilOp::KEEP;
  EStencilOp DepthFailOp = EStencilOp::KEEP;
  ECompareOp CompareOp   = ECompareOp::NEVER;
  uint32_t   CompareMask = 0;
  uint32_t   WriteMask   = 0;
  uint32_t   Reference   = 0;
};

//////////////////////////////////////////////////////////////////////////
// DepthStencilStateInfo
//////////////////////////////////////////////////////////////////////////
struct DepthStencilStateInfo {
  virtual ~DepthStencilStateInfo() {}

  virtual void Initialize() { GetHash(); }

  virtual size_t GetHash() const {
    if (Hash) {
      return Hash;
    }

    Hash = GETHASH_FROM_INSTANT_STRUCT(DepthTestEnable,
                                       DepthWriteEnable,
                                       DepthCompareOp,
                                       DepthBoundsTestEnable,
                                       StencilTestEnable,
                                       (Front ? Front->GetHash() : 0),
                                       (Back ? Back->GetHash() : 0),
                                       MinDepthBounds,
                                       MaxDepthBounds);
    return Hash;
  }

  mutable size_t Hash = 0;

  bool                DepthTestEnable       = false;
  bool                DepthWriteEnable      = false;
  ECompareOp          DepthCompareOp        = ECompareOp::LEQUAL;
  bool                DepthBoundsTestEnable = false;
  bool                StencilTestEnable     = false;
  StencilOpStateInfo* Front                 = nullptr;
  StencilOpStateInfo* Back                  = nullptr;
  float               MinDepthBounds        = 0.0f;
  float               MaxDepthBounds        = 1.0f;

  // VkPipelineDepthStencilStateCreateFlags    flags;
};

//////////////////////////////////////////////////////////////////////////
// BlendingStateInfo
//////////////////////////////////////////////////////////////////////////
struct BlendingStateInfo {
  virtual ~BlendingStateInfo() {}

  virtual void Initialize() { GetHash(); }

  virtual size_t GetHash() const {
    if (Hash) {
      return Hash;
    }

    Hash = GETHASH_FROM_INSTANT_STRUCT(BlendEnable,
                                       Src,
                                       Dest,
                                       BlendOp,
                                       SrcAlpha,
                                       DestAlpha,
                                       AlphaBlendOp,
                                       ColorWriteMask);
    return Hash;
  }

  mutable size_t Hash = 0;

  bool         BlendEnable    = false;
  EBlendFactor Src            = EBlendFactor::SRC_COLOR;
  EBlendFactor Dest           = EBlendFactor::ONE_MINUS_SRC_ALPHA;
  EBlendOp     BlendOp        = EBlendOp::ADD;
  EBlendFactor SrcAlpha       = EBlendFactor::SRC_ALPHA;
  EBlendFactor DestAlpha      = EBlendFactor::ONE_MINUS_SRC_ALPHA;
  EBlendOp     AlphaBlendOp   = EBlendOp::ADD;
  EColorMask   ColorWriteMask = EColorMask::NONE;

  // VkPipelineColorBlendStateCreateFlags          flags;
  // VkBool32                                      logicOpEnable;
  // VkLogicOp                                     logicOp;
  // uint32_t                                      attachmentCount;
  // const VkPipelineColorBlendAttachmentState* pAttachments;
  // float                                         blendConstants[4];
};

//////////////////////////////////////////////////////////////////////////
// PipelineStateFixedInfo
//////////////////////////////////////////////////////////////////////////
struct PipelineStateFixedInfo {
  PipelineStateFixedInfo() = default;

  PipelineStateFixedInfo(RasterizationStateInfo*      rasterizationState,
                         DepthStencilStateInfo*       depthStencilState,
                         BlendingStateInfo*           blendingState,
                         const std::vector<Viewport>& viewports,
                         const std::vector<Scissor>&  scissors,
                         bool                         isUseVRS)
      : RasterizationState(rasterizationState)
      , DepthStencilState(depthStencilState)
      , BlendingState(blendingState)
      , viewports(viewports)
      , scissors(scissors)
      , IsUseVRS(isUseVRS) {
    CreateHash();
  }

  PipelineStateFixedInfo(RasterizationStateInfo* rasterizationState,
                         DepthStencilStateInfo*  depthStencilState,
                         BlendingStateInfo*      blendingState,
                         const Viewport&         viewport,
                         const Scissor&          scissor,
                         bool                    isUseVRS)
      : RasterizationState(rasterizationState)
      , DepthStencilState(depthStencilState)
      , BlendingState(blendingState)
      , viewports({viewport})
      , scissors({scissor})
      , IsUseVRS(isUseVRS) {
    CreateHash();
  }

  PipelineStateFixedInfo(
      RasterizationStateInfo*                   rasterizationState,
      DepthStencilStateInfo*                    depthStencilState,
      BlendingStateInfo*                        blendingState,
      const std::vector<EPipelineDynamicState>& InDynamicStates,
      bool                                      isUseVRS)
      : RasterizationState(rasterizationState)
      , DepthStencilState(depthStencilState)
      , BlendingState(blendingState)
      , DynamicStates(InDynamicStates)
      , IsUseVRS(isUseVRS) {
    CreateHash();
  }

  size_t CreateHash() const {
    if (Hash) {
      return Hash;
    }

    Hash = 0;
    for (int32_t i = 0; i < viewports.size(); ++i) {
      Hash ^= (viewports[i].GetHash() ^ (i + 1));
    }

    for (int32_t i = 0; i < scissors.size(); ++i) {
      Hash ^= (scissors[i].GetHash() ^ (i + 1));
    }

    if (DynamicStates.size() > 0) {
      Hash = ::XXH64(&DynamicStates[0],
                     sizeof(EPipelineDynamicState) * DynamicStates.size(),
                     Hash);
    }

    // TODO: The contents below should also be able to generate a hash
    Hash ^= RasterizationState->GetHash();
    Hash ^= DepthStencilState->GetHash();
    Hash ^= BlendingState->GetHash();
    Hash ^= (uint64_t)IsUseVRS;

    return Hash;
  }

  std::vector<Viewport>              viewports;
  std::vector<Scissor>               scissors;
  std::vector<EPipelineDynamicState> DynamicStates;

  RasterizationStateInfo* RasterizationState = nullptr;
  DepthStencilStateInfo*  DepthStencilState  = nullptr;
  BlendingStateInfo*      BlendingState      = nullptr;
  bool                    IsUseVRS           = false;

  mutable size_t Hash = 0;
};

struct PushConstantRange {
  PushConstantRange() = default;

  PushConstantRange(EShaderAccessStageFlag accessStageFlag,
                    int32_t                offset,
                    int32_t                size)
      : AccessStageFlag(accessStageFlag)
      , Offset(offset)
      , Size(size) {}

  EShaderAccessStageFlag AccessStageFlag = EShaderAccessStageFlag::ALL_GRAPHICS;
  int32_t                Offset          = 0;
  int32_t                Size            = 0;
};

struct PushConstant {
  PushConstant() = default;

  PushConstant(const PushConstant& InPushConstant) {
    assert(InPushConstant.UsedSize < 256);

    UsedSize = InPushConstant.UsedSize;
    memcpy(Data, InPushConstant.Data, InPushConstant.UsedSize);
    PushConstantRanges = InPushConstant.PushConstantRanges;
    Hash               = InPushConstant.Hash;
  }

  PushConstant(const char*            InData,
               int32_t                InSize,
               EShaderAccessStageFlag InShaderAccessStageFlag) {
    assert(InSize < 256);

    UsedSize = InSize;
    memcpy(Data, InData, InSize);
    PushConstantRanges.Add(
        PushConstantRange(InShaderAccessStageFlag, 0, InSize));
    GetHash();
  }

  PushConstant(const char*              InData,
               int32_t                  InSize,
               const PushConstantRange& InPushConstantRange) {
    assert(InSize < 256);

    UsedSize = InSize;
    memcpy(Data, InData, InSize);
    PushConstantRanges.Add(InPushConstantRange);
    GetHash();
  }

  PushConstant(const char*                                 InData,
               int32_t                                     InSize,
               const ResourceContainer<PushConstantRange>& InPushConstantRanges)
      : PushConstantRanges(InPushConstantRanges) {
    assert(InSize < 256);

    UsedSize = InSize;
    memcpy(Data, InData, InSize);
    GetHash();
  }

  template <typename T>
  PushConstant(const T&               InData,
               EShaderAccessStageFlag InShaderAccessStageFlag) {
    Set(InData, PushConstantRange(InShaderAccessStageFlag, 0, sizeof(T)));
  }

  template <typename T>
  PushConstant(const T& InData, const PushConstantRange& InPushConstantRange) {
    Set(InData, InPushConstantRange);
  }

  template <typename T>
  PushConstant(
      const T&                                    InData,
      const ResourceContainer<PushConstantRange>& InPushConstantRanges) {
    Set(InData, InPushConstantRanges);
  }

  template <typename T>
  void Set(const T& InData, const PushConstantRange& InPushConstantRange) {
    assert(sizeof(T) < 256);

    UsedSize = sizeof(T);
    memcpy(Data, &InData, sizeof(T));
    PushConstantRanges.Add(InPushConstantRange);
    GetHash();
  }

  template <typename T>
  void Set(const T&                                    InData,
           const ResourceContainer<PushConstantRange>& InPushConstantRanges) {
    assert(sizeof(T) < 256);

    UsedSize = sizeof(T);
    memcpy(Data, &InData, sizeof(T));
    PushConstantRanges = InPushConstantRanges;
    GetHash();
  }

  template <typename T>
  T& Get() const {
    return *(T*)&Data[0];
  }

  bool IsValid() const { return UsedSize > 0; }

  PushConstant& operator=(const PushConstant& InPushConstant) {
    assert(InPushConstant.UsedSize < 256);

    UsedSize = InPushConstant.UsedSize;
    memcpy(Data, InPushConstant.Data, InPushConstant.UsedSize);
    PushConstantRanges = InPushConstant.PushConstantRanges;
    Hash               = InPushConstant.Hash;
    return *this;
  }

  size_t GetHash() const {
    if (Hash) {
      return Hash;
    }

    Hash = 0;
    if (PushConstantRanges.NumOfData > 0) {
      // TODO: be careful with sizeof(PushConstantRange) when added
      // incapsulation
      Hash = ::XXH64(&PushConstantRanges[0],
                     sizeof(PushConstantRange) * PushConstantRanges.NumOfData,
                     Hash);
    }

    return Hash;
  }

  const void* GetConstantData() const { return (void*)&Data[0]; }

  int32_t GetSize() const { return UsedSize; }

  const ResourceContainer<PushConstantRange>* GetPushConstantRanges() const {
    return &PushConstantRanges;
  }

  mutable size_t                       Hash = 0;
  ResourceContainer<PushConstantRange> PushConstantRanges;
  uint8_t                              Data[256];
  int32_t                              UsedSize = 0;
};

// struct Shader;
// struct GraphicsPipelineShader;
// struct VertexBuffer;
// struct ShaderBindingLayout;
// class  m_renderPass;
// struct RenderFrameContext;

//////////////////////////////////////////////////////////////////////////
// PipelineStateInfo
//////////////////////////////////////////////////////////////////////////
struct PipelineStateInfo {
  enum class EPipelineType : uint8_t {
    Graphics = 0,
    Compute,
    RayTracing
  };

  PipelineStateInfo() = default;

  PipelineStateInfo(const PipelineStateFixedInfo*   InPipelineStateFixed,
                    const GraphicsPipelineShader    InShader,
                    const VertexBufferArray&        InVertexBufferArray,
                    const RenderPass*              InRenderPass,
                    const ShaderBindingLayoutArray& InShaderBindingLayoutArray,
                    const PushConstant*             InPushConstant = nullptr,
                    int32_t                         InSubpassIndex = 0)
      : PipelineStateFixed(InPipelineStateFixed)
      , GraphicsShader(InShader)
      , m_vertexBufferArray(InVertexBufferArray)
      , m_renderPass(InRenderPass)
      , m_shaderBindingLayoutArray(InShaderBindingLayoutArray)
      , m_pushConstant(InPushConstant)
      , SubpassIndex(InSubpassIndex) {
    PipelineType = EPipelineType::Graphics;
  }

  PipelineStateInfo(const Shader*                   InComputeShader,
                    const ShaderBindingLayoutArray& InShaderBindingLayoutArray,
                    const PushConstant*             InPushConstant = nullptr,
                    int32_t                         InSubpassIndex = 0)
      : ComputeShader(InComputeShader)
      , m_shaderBindingLayoutArray(InShaderBindingLayoutArray)
      , m_pushConstant(InPushConstant)
      , SubpassIndex(InSubpassIndex) {
    PipelineType = EPipelineType::Compute;
  }

  // PipelineStateInfo(
  //     const std::vector<RaytracingPipelineShader>& InShader,
  //     const RaytracingPipelineData&                InRaytracingPipelineData,
  //     const ShaderBindingLayoutArray& InShaderBindingLayoutArray, const
  //     PushConstant*                          InPushConstant = nullptr,
  //     int32_t                                       InSubpassIndex = 0)
  //     : RaytracingShaders(InShader)
  //     , RaytracingPipelineData(InRaytracingPipelineData)
  //     , m_shaderBindingLayoutArray(InShaderBindingLayoutArray)
  //     , m_pushConstant(InPushConstant)
  //     , SubpassIndex(InSubpassIndex) {
  //   PipelineType = EPipelineType::RayTracing;
  // }

  PipelineStateInfo(const PipelineStateInfo& InPipelineState)
      : PipelineStateFixed(InPipelineState.PipelineStateFixed)
      , GraphicsShader(InPipelineState.GraphicsShader)
      , ComputeShader(InPipelineState.ComputeShader)
      //, RaytracingShaders(InPipelineState.RaytracingShaders)
      , PipelineType(InPipelineState.PipelineType)
      , m_vertexBufferArray(InPipelineState.m_vertexBufferArray)
      , m_renderPass(InPipelineState.m_renderPass)
      , m_shaderBindingLayoutArray(InPipelineState.m_shaderBindingLayoutArray)
      , m_pushConstant(InPipelineState.m_pushConstant)
      , Hash(InPipelineState.Hash)
      , SubpassIndex(InPipelineState.SubpassIndex)
  //, RaytracingPipelineData(InPipelineState.RaytracingPipelineData)
  {}

  PipelineStateInfo(PipelineStateInfo&& InPipelineState) noexcept
      : PipelineStateFixed(InPipelineState.PipelineStateFixed)
      , GraphicsShader(InPipelineState.GraphicsShader)
      , ComputeShader(InPipelineState.ComputeShader)
      //, RaytracingShaders(InPipelineState.RaytracingShaders)
      , PipelineType(InPipelineState.PipelineType)
      , m_vertexBufferArray(InPipelineState.m_vertexBufferArray)
      , m_renderPass(InPipelineState.m_renderPass)
      , m_shaderBindingLayoutArray(InPipelineState.m_shaderBindingLayoutArray)
      , m_pushConstant(InPipelineState.m_pushConstant)
      , Hash(InPipelineState.Hash)
      , SubpassIndex(InPipelineState.SubpassIndex)
  //, RaytracingPipelineData(InPipelineState.RaytracingPipelineData)
  {}

  virtual ~PipelineStateInfo() {}

  size_t GetHash() const;

  mutable size_t Hash = 0;

  EPipelineType                 PipelineType = EPipelineType::Graphics;
  const GraphicsPipelineShader  GraphicsShader;
  const Shader*                 ComputeShader = nullptr;
  // std::vector<RaytracingPipelineShader> RaytracingShaders;
  // RaytracingPipelineData                RaytracingPipelineData;
  const RenderPass*            m_renderPass = nullptr;
  VertexBufferArray             m_vertexBufferArray;
  ShaderBindingLayoutArray      m_shaderBindingLayoutArray;
  const PushConstant*           m_pushConstant;
  const PipelineStateFixedInfo* PipelineStateFixed = nullptr;
  int32_t                       SubpassIndex       = 0;

  virtual void Initialize() { GetHash(); }

  virtual void* GetHandle() const { return nullptr; }

  virtual void* GetPipelineLayoutHandle() const { return nullptr; }

  virtual void* CreateGraphicsPipelineState() { return nullptr; }

  virtual void* CreateComputePipelineState() { return nullptr; }

  virtual void* CreateRaytracingPipelineState() { return nullptr; }

  virtual void Bind(
      const std::shared_ptr<RenderFrameContext>& InRenderFrameContext) const {}
};

}  // namespace game_engine

#endif  // GAME_ENGINE_PIPELINE_STATE_INFO_H