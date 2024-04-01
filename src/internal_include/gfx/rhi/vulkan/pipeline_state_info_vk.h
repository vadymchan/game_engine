#ifndef GAME_ENGINE_PIPELINE_STATE_INFO_VK_H
#define GAME_ENGINE_PIPELINE_STATE_INFO_VK_H

#include "gfx/rhi/resource_container.h"
#include "gfx/rhi/shader_bindable_resource.h"
#include "gfx/rhi/vulkan/shader_binding_layout_vk.h"
#include "gfx/rhi/vulkan/buffer_vk.h"
#include "gfx/rhi/vulkan/render_pass_vk.h"
#include "gfx/rhi/vulkan/render_target_vk.h"
#include "gfx/rhi/vulkan/shader_vk.h"

#include <math_library/dimension.h>
#include <vulkan/vulkan.h>

#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

namespace game_engine {

// TODO: consider moving to other file
constexpr bool useVulkanNdcYFlip = true;

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
// SamplerStateInfoVk
//////////////////////////////////////////////////////////////////////////
struct SamplerStateInfoVk : public ShaderBindableResource {
  SamplerStateInfoVk() = default;

  virtual ~SamplerStateInfoVk() { Release(); }

  virtual void Initialize();

  void Release();

  virtual void* GetHandle() const { return SamplerState; }

  virtual size_t GetHash() const;

  std::string ToString() const;

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
  float MinLOD = -FLT_MAX;  // TODO: consider change default value to 0.0f
  float MaxLOD = FLT_MAX;

  VkSamplerCreateInfo SamplerStateInfo = {};
  VkSampler           SamplerState     = nullptr;
};



//////////////////////////////////////////////////////////////////////////
// RasterizationStateInfoVk
//////////////////////////////////////////////////////////////////////////
struct RasterizationStateInfoVk {
  RasterizationStateInfoVk() = default;

  virtual ~RasterizationStateInfoVk() {}

  virtual void Initialize();

  virtual size_t GetHash() const;

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

  EMSAASamples SampleCount           = EMSAASamples::COUNT_1;
  bool         SampleShadingEnable   = true;
  float        MinSampleShading      = 0.2f;
  bool         AlphaToCoverageEnable = false;
  bool         AlphaToOneEnable      = false;

  VkPipelineRasterizationStateCreateInfo RasterizationStateInfo = {};
  VkPipelineMultisampleStateCreateInfo   MultisampleStateInfo   = {};
};



//////////////////////////////////////////////////////////////////////////
// StencilOpStateInfoVk
//////////////////////////////////////////////////////////////////////////
struct StencilOpStateInfoVk {
  virtual ~StencilOpStateInfoVk() {}

  virtual void Initialize();

  virtual size_t GetHash() const;

  mutable size_t Hash = 0;

  EStencilOp FailOp      = EStencilOp::KEEP;
  EStencilOp PassOp      = EStencilOp::KEEP;
  EStencilOp DepthFailOp = EStencilOp::KEEP;
  ECompareOp CompareOp   = ECompareOp::NEVER;
  uint32_t   CompareMask = 0;
  uint32_t   WriteMask   = 0;
  uint32_t   Reference   = 0;

  VkStencilOpState StencilOpStateInfo = {};
};

//////////////////////////////////////////////////////////////////////////
// DepthStencilStateInfoVk
//////////////////////////////////////////////////////////////////////////
struct DepthStencilStateInfoVk {
  virtual ~DepthStencilStateInfoVk() {}

  virtual void Initialize();

  virtual size_t GetHash() const;

  mutable size_t Hash = 0;

  bool                  DepthTestEnable       = false;
  bool                  DepthWriteEnable      = false;
  ECompareOp            DepthCompareOp        = ECompareOp::LEQUAL;
  bool                  DepthBoundsTestEnable = false;
  bool                  StencilTestEnable     = false;
  StencilOpStateInfoVk* Front                 = nullptr;
  StencilOpStateInfoVk* Back                  = nullptr;
  float                 MinDepthBounds        = 0.0f;
  float                 MaxDepthBounds        = 1.0f;

  // VkPipelineDepthStencilStateCreateFlags    flags;
  VkPipelineDepthStencilStateCreateInfo DepthStencilStateInfo = {};
};



//////////////////////////////////////////////////////////////////////////
// BlendingStateInfoVk
//////////////////////////////////////////////////////////////////////////
struct BlendingStateInfoVk {
  virtual ~BlendingStateInfoVk() {}

  virtual void Initialize();

  virtual size_t GetHash() const;

  mutable size_t Hash = 0;

  bool BlendEnable = false;

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
  // uint32_t                                       attachmentCount;
  // const VkPipelineColorBlendAttachmentState* pAttachments;
  // float                                         blendConstants[4];

  VkPipelineColorBlendAttachmentState ColorBlendAttachmentInfo = {};
};



//////////////////////////////////////////////////////////////////////////
// PipelineStateFixedInfoVk
//////////////////////////////////////////////////////////////////////////
struct PipelineStateFixedInfoVk {
  PipelineStateFixedInfoVk() = default;

  PipelineStateFixedInfoVk(RasterizationStateInfoVk*    rasterizationState,
                           DepthStencilStateInfoVk*     depthStencilState,
                           BlendingStateInfoVk*         blendingState,
                           const std::vector<Viewport>& viewports,
                           const std::vector<Scissor>&  scissors,
                           bool                         isUseVRS)
      : RasterizationState(rasterizationState)
      , DepthStencilState(depthStencilState)
      , BlendingState(blendingState)
      , Viewports(Viewports)
      , Scissors(scissors)
      , IsUseVRS(isUseVRS) {
    CreateHash();
  }

  PipelineStateFixedInfoVk(RasterizationStateInfoVk* rasterizationState,
                           DepthStencilStateInfoVk*  depthStencilState,
                           BlendingStateInfoVk*      blendingState,
                           const Viewport&           viewport,
                           const Scissor&            scissor,
                           bool                      isUseVRS)
      : RasterizationState(rasterizationState)
      , DepthStencilState(depthStencilState)
      , BlendingState(blendingState)
      , Viewports({viewport})
      , Scissors({scissor})
      , IsUseVRS(isUseVRS) {
    CreateHash();
  }

  PipelineStateFixedInfoVk(
      RasterizationStateInfoVk*                 rasterizationState,
      DepthStencilStateInfoVk*                  depthStencilState,
      BlendingStateInfoVk*                      blendingState,
      const std::vector<EPipelineDynamicState>& InDynamicStates,
      bool                                      isUseVRS)
      : RasterizationState(rasterizationState)
      , DepthStencilState(depthStencilState)
      , BlendingState(blendingState)
      , DynamicStates(InDynamicStates)
      , IsUseVRS(isUseVRS) {
    CreateHash();
  }

  size_t CreateHash() const;

  std::vector<Viewport>              Viewports;
  std::vector<Scissor>               Scissors;
  std::vector<EPipelineDynamicState> DynamicStates;

  RasterizationStateInfoVk* RasterizationState = nullptr;
  DepthStencilStateInfoVk*  DepthStencilState  = nullptr;
  BlendingStateInfoVk*      BlendingState      = nullptr;
  bool                      IsUseVRS           = false;

  mutable size_t Hash = 0;
};

struct PushConstantRangeVk {
  PushConstantRangeVk() = default;

  PushConstantRangeVk(EShaderAccessStageFlag accessStageFlag,
                      int32_t                offset,
                      int32_t                size)
      : AccessStageFlag(accessStageFlag)
      , Offset(offset)
      , Size(size) {}

  EShaderAccessStageFlag AccessStageFlag = EShaderAccessStageFlag::ALL_GRAPHICS;
  int32_t                Offset          = 0;
  int32_t                Size            = 0;
};

struct PushConstantVk {
  PushConstantVk() = default;

  PushConstantVk(const PushConstantVk& InPushConstant);

  PushConstantVk(const char*            InData,
                 int32_t                InSize,
                 EShaderAccessStageFlag InShaderAccessStageFlag);

  PushConstantVk(const char*                InData,
                 int32_t                    InSize,
                 const PushConstantRangeVk& InPushConstantRange);

  PushConstantVk(
      const char*                                   InData,
      int32_t                                       InSize,
      const ResourceContainer<PushConstantRangeVk>& InPushConstantRanges);

  template <typename T>
  PushConstantVk(const T&               InData,
                 EShaderAccessStageFlag InShaderAccessStageFlag) {
    Set(InData, PushConstantRangeVk(InShaderAccessStageFlag, 0, sizeof(T)));
  }

  template <typename T>
  PushConstantVk(const T&                   InData,
                 const PushConstantRangeVk& InPushConstantRange) {
    Set(InData, InPushConstantRange);
  }

  template <typename T>
  PushConstantVk(
      const T&                                      InData,
      const ResourceContainer<PushConstantRangeVk>& InPushConstantRanges) {
    Set(InData, InPushConstantRanges);
  }

  template <typename T>
  void Set(const T& InData, const PushConstantRangeVk& InPushConstantRange) {
    assert(sizeof(T) < 256);

    UsedSize = sizeof(T);
    memcpy(Data, &InData, sizeof(T));
    PushConstantRanges.Add(InPushConstantRange);
    GetHash();
  }

  template <typename T>
  void Set(const T&                                      InData,
           const ResourceContainer<PushConstantRangeVk>& InPushConstantRanges) {
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

  PushConstantVk& operator=(const PushConstantVk& InPushConstant);

  size_t GetHash() const;

  const void* GetConstantData() const { return (void*)&Data[0]; }

  int32_t GetSize() const { return UsedSize; }

  const ResourceContainer<PushConstantRangeVk>* GetPushConstantRanges() const {
    return &PushConstantRanges;
  }

  mutable size_t                         Hash = 0;
  ResourceContainer<PushConstantRangeVk> PushConstantRanges;
  uint8_t                                Data[256];
  int32_t                                UsedSize = 0;
};

//////////////////////////////////////////////////////////////////////////
// PipelineStateInfoVk
//////////////////////////////////////////////////////////////////////////

struct PipelineStateInfoVk {
  enum class EPipelineType : uint8_t {
    Graphics = 0,
    Compute,
    RayTracing
  };

  PipelineStateInfoVk() = default;

  PipelineStateInfoVk(
      const PipelineStateFixedInfoVk*   InPipelineStateFixed,
      const GraphicsPipelineShader      InShader,
      const VertexBufferArrayVk&        InVertexBufferArray,
      const RenderPassVk*               InRenderPass,
      const ShaderBindingLayoutArrayVk& InShaderBindingLayoutArray,
      const PushConstantVk*             InPushConstant = nullptr,
      int32_t                           InSubpassIndex = 0)
      : PipelineStateFixed(InPipelineStateFixed)
      , GraphicsShader(InShader)
      , VertexBufferArray(InVertexBufferArray)
      , RenderPass(InRenderPass)
      , ShaderBindingLayoutArray(InShaderBindingLayoutArray)
      , PushConstant(InPushConstant)
      , SubpassIndex(InSubpassIndex) {
    PipelineType = EPipelineType::Graphics;
  }

  PipelineStateInfoVk(
      const Shader*                     InComputeShader,
      const ShaderBindingLayoutArrayVk& InShaderBindingLayoutArray,
      const PushConstantVk*             InPushConstant = nullptr,
      int32_t                           InSubpassIndex = 0)
      : ComputeShader(InComputeShader)
      , ShaderBindingLayoutArray(InShaderBindingLayoutArray)
      , PushConstant(InPushConstant)
      , SubpassIndex(InSubpassIndex) {
    PipelineType = EPipelineType::Compute;
  }

  PipelineStateInfoVk(const PipelineStateInfoVk& InPipelineState)
      : PipelineStateFixed(InPipelineState.PipelineStateFixed)
      , GraphicsShader(InPipelineState.GraphicsShader)
      , ComputeShader(InPipelineState.ComputeShader)
      , PipelineType(InPipelineState.PipelineType)
      , VertexBufferArray(InPipelineState.VertexBufferArray)
      , RenderPass(InPipelineState.RenderPass)
      , ShaderBindingLayoutArray(InPipelineState.ShaderBindingLayoutArray)
      , PushConstant(InPipelineState.PushConstant)
      , Hash(InPipelineState.Hash)
      , SubpassIndex(InPipelineState.SubpassIndex) {}

  PipelineStateInfoVk(PipelineStateInfoVk&& InPipelineState) noexcept
      : PipelineStateFixed(InPipelineState.PipelineStateFixed)
      , GraphicsShader(InPipelineState.GraphicsShader)
      , ComputeShader(InPipelineState.ComputeShader)
      , PipelineType(InPipelineState.PipelineType)
      , VertexBufferArray(InPipelineState.VertexBufferArray)
      , RenderPass(InPipelineState.RenderPass)
      , ShaderBindingLayoutArray(InPipelineState.ShaderBindingLayoutArray)
      , PushConstant(InPipelineState.PushConstant)
      , Hash(InPipelineState.Hash)
      , SubpassIndex(InPipelineState.SubpassIndex) {}

  virtual ~PipelineStateInfoVk() { Release(); }

  virtual void Initialize();

  void Release();

  size_t GetHash() const;

  virtual void* GetHandle() const { return vkPipeline; }

  virtual void* GetPipelineLayoutHandle() const { return vkPipelineLayout; }

  virtual void* CreateGraphicsPipelineState();

  virtual void* CreateComputePipelineState();

  virtual void Bind(
      const std::shared_ptr<RenderFrameContextVk>& InRenderFrameContext) const;

  mutable size_t Hash = 0;

  EPipelineType                   PipelineType = EPipelineType::Graphics;
  const GraphicsPipelineShader    GraphicsShader;
  const Shader*                   ComputeShader = nullptr;
  const RenderPassVk*             RenderPass    = nullptr;
  VertexBufferArrayVk             VertexBufferArray;
  ShaderBindingLayoutArrayVk      ShaderBindingLayoutArray;
  const PushConstantVk*           PushConstant;
  const PipelineStateFixedInfoVk* PipelineStateFixed = nullptr;
  int32_t                         SubpassIndex       = 0;
  VkPipeline                      vkPipeline         = nullptr;
  VkPipelineLayout                vkPipelineLayout   = nullptr;
};

}  // namespace game_engine


#endif  // GAME_ENGINE_PIPELINE_STATE_INFO_VK_H
