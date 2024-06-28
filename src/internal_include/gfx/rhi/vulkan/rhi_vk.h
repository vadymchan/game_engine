
// TODO: add utils class for functions and variables

#ifndef GAME_ENGINE_RHI_VK_H
#define GAME_ENGINE_RHI_VK_H

#ifndef NDEBUG  // DEBUG
  #define ENABLE_VALIDATION_LAYERS
#endif

#define VALIDATION_LAYER_VERBOSE 0

#include "file_loader/file.h"
#include "file_loader/image_file_loader.h"
#include "gfx/renderer/material.h"
#include "gfx/rhi/lock.h"
#include "gfx/rhi/resource_pool.h"
#include "gfx/rhi/rhi.h"
#include "gfx/rhi/vulkan/command_pool_vk.h"
#include "gfx/rhi/vulkan/descriptor_pool_vk.h"
#include "gfx/rhi/vulkan/frame_buffer_pool_vk.h"
#include "gfx/rhi/vulkan/memory_pool_vk.h"
#include "gfx/rhi/vulkan/pipeline_state_info_vk.h"
#include "gfx/rhi/vulkan/render_frame_context_vk.h"
#include "gfx/rhi/vulkan/render_target_pool_vk.h"
#include "gfx/rhi/vulkan/render_target_vk.h"
#include "gfx/rhi/vulkan/rhi_type_vk.h"
#include "gfx/rhi/vulkan/ring_buffer_vk.h"
#include "gfx/rhi/vulkan/semaphore_vk.h"
#include "gfx/rhi/vulkan/swapchain_vk.h"
#include "gfx/rhi/vulkan/uniform_buffer_object_vk.h"
#include "gfx/rhi/vulkan/utils_vk.h"
#include "platform/common/window.h"
#include "utils/logger/global_logger.h"

// TODO: move these vulkan SDK includes to separate file
#include <SDL_vulkan.h>
#include <vulkan/vulkan.h>

#include <set>
#include <vector>

namespace game_engine {

class RhiVk : public jRHI {
  public:
  RhiVk();
  RhiVk(const RhiVk&)                    = delete;
  auto operator=(const RhiVk&) -> RhiVk& = delete;
  RhiVk(RhiVk&&)                         = delete;
  auto operator=(RhiVk&&) -> RhiVk&      = delete;
  virtual ~RhiVk()                       = default;

  // TODO: consider change signature for Window object
  virtual bool init(const std::shared_ptr<Window>& window) override;

  virtual void release() override;

  virtual std::shared_ptr<jVertexBuffer> CreateVertexBuffer(
      const std::shared_ptr<VertexStreamData>& streamData) const override;

  virtual std::shared_ptr<jIndexBuffer> CreateIndexBuffer(
      const std::shared_ptr<IndexStreamData>& streamData) const override;

  virtual std::shared_ptr<jTexture> CreateTextureFromData(
      const ImageData* InImageData) const override;

  virtual bool CreateShaderInternal(
      Shader* OutShader, const ShaderInfo& shaderInfo) const override;

  virtual jFrameBuffer* CreateFrameBuffer(
      const jFrameBufferInfo& info) const override;

  virtual std::shared_ptr<jRenderTarget> CreateRenderTarget(
      const jRenderTargetInfo& info) const override;

  virtual jSamplerStateInfo* CreateSamplerState(
      const jSamplerStateInfo& initializer) const override {
    return SamplerStatePool.GetOrCreate(initializer);
  }

  virtual jRasterizationStateInfo* CreateRasterizationState(
      const jRasterizationStateInfo& initializer) const override {
    return RasterizationStatePool.GetOrCreate(initializer);
  }

  virtual jStencilOpStateInfo* CreateStencilOpStateInfo(
      const jStencilOpStateInfo& initializer) const override {
    return StencilOpStatePool.GetOrCreate(initializer);
  }

  virtual jDepthStencilStateInfo* CreateDepthStencilState(
      const jDepthStencilStateInfo& initializer) const override {
    return DepthStencilStatePool.GetOrCreate(initializer);
  }

  virtual jBlendingStateInfo* CreateBlendingState(
      const jBlendingStateInfo& initializer) const override {
    return BlendingStatePool.GetOrCreate(initializer);
  }

  virtual jShaderBindingLayout* CreateShaderBindings(
      const jShaderBindingArray& InShaderBindingArray) const override;

  virtual std::shared_ptr<jShaderBindingInstance> CreateShaderBindingInstance(
      const jShaderBindingArray&       InShaderBindingArray,
      const jShaderBindingInstanceType InType) const override;

  virtual jPipelineStateInfo* CreatePipelineStateInfo(
      const jPipelineStateFixedInfo*   pipelineStateFixed,
      const GraphicsPipelineShader     shader,
      const jVertexBufferArray&        InVertexBufferArray,
      const jRenderPass*               renderPass,
      const jShaderBindingLayoutArray& InShaderBindingArray,
      const jPushConstant*             InPushConstant,
      std::int32_t                     InSubpassIndex) const override {
    return PipelineStatePool.GetOrCreateMove(
        std::move(jPipelineStateInfo(pipelineStateFixed,
                                     shader,
                                     InVertexBufferArray,
                                     renderPass,
                                     InShaderBindingArray,
                                     InPushConstant,
                                     InSubpassIndex)));
  }

  virtual jPipelineStateInfo* CreateComputePipelineStateInfo(
      const Shader*                    shader,
      const jShaderBindingLayoutArray& InShaderBindingArray,
      const jPushConstant*             pushConstant) const override {
    return PipelineStatePool.GetOrCreateMove(std::move(
        jPipelineStateInfo(shader, InShaderBindingArray, pushConstant)));
  }

  // Create Buffers
  std::shared_ptr<jBuffer> CreateBufferInternal(
      std::uint64_t     InSize,
      std::uint64_t     InAlignment,
      EBufferCreateFlag InBufferCreateFlag,
      EResourceLayout   InInitialState,
      const void*       InData     = nullptr,
      std::uint64_t     InDataSize = 0
      /*, const wchar_t*    InResourceName = nullptr*/) const;

  virtual std::shared_ptr<jBuffer> CreateStructuredBuffer(
      std::uint64_t     InSize,
      std::uint64_t     InAlignment,
      std::uint64_t     InStride,
      EBufferCreateFlag InBufferCreateFlag,
      EResourceLayout   InInitialState,
      const void*       InData     = nullptr,
      std::uint64_t     InDataSize = 0) const override {
    return CreateBufferInternal(InSize,
                                InAlignment,
                                InBufferCreateFlag,
                                InInitialState,
                                InData,
                                InDataSize);
  }

  virtual std::shared_ptr<jBuffer> CreateRawBuffer(
      std::uint64_t     InSize,
      std::uint64_t     InAlignment,
      EBufferCreateFlag InBufferCreateFlag,
      EResourceLayout   InInitialState,
      const void*       InData     = nullptr,
      std::uint64_t     InDataSize = 0) const override {
    return CreateBufferInternal(InSize,
                                InAlignment,
                                InBufferCreateFlag,
                                InInitialState,
                                InData,
                                InDataSize);
  }

  virtual std::shared_ptr<jBuffer> CreateFormattedBuffer(
      std::uint64_t     InSize,
      std::uint64_t     InAlignment,
      ETextureFormat    InFormat,
      EBufferCreateFlag InBufferCreateFlag,
      EResourceLayout   InInitialState,
      const void*       InData     = nullptr,
      std::uint64_t     InDataSize = 0) const override {
    return CreateBufferInternal(InSize,
                                InAlignment,
                                InBufferCreateFlag,
                                InInitialState,
                                InData,
                                InDataSize);
  }

  virtual std::shared_ptr<IUniformBufferBlock> CreateUniformBufferBlock(
      Name         InName,
      LifeTimeType InLifeTimeType,
      size_t       InSize = 0) const override;

  // Create Images
  VkImageUsageFlags GetImageUsageFlags(
      ETextureCreateFlag InTextureCreateFlag) const;

  VkMemoryPropertyFlagBits GetMemoryPropertyFlagBits(
      ETextureCreateFlag InTextureCreateFlag) const;

  virtual std::shared_ptr<jTexture> Create2DTexture(
      uint32_t             InWidth,
      uint32_t             InHeight,
      uint32_t             InArrayLayers,
      uint32_t             InMipLevels,
      ETextureFormat       InFormat,
      ETextureCreateFlag   InTextureCreateFlag,
      EResourceLayout      InImageLayout   = EResourceLayout::UNDEFINED,
      const ImageBulkData& InImageBulkData = {},
      const jRTClearValue& InClearValue    = jRTClearValue::Invalid,
      const wchar_t*       InResourceName  = nullptr) const override;

  virtual std::shared_ptr<jTexture> CreateCubeTexture(
      uint32_t             InWidth,
      uint32_t             InHeight,
      uint32_t             InMipLevels,
      ETextureFormat       InFormat,
      ETextureCreateFlag   InTextureCreateFlag,
      EResourceLayout      InImageLayout   = EResourceLayout::UNDEFINED,
      const ImageBulkData& InImageBulkData = {},
      const jRTClearValue& InClearValue    = jRTClearValue::Invalid,
      const wchar_t*       InResourceName  = nullptr) const override;

  void RemovePipelineStateInfo(size_t InHash) {
    PipelineStatePool.Release(InHash);
  }

  virtual jRenderPass* GetOrCreateRenderPass(
      const std::vector<jAttachment>& colorAttachments,
      const math::Vector2Di&          offset,
      const math::Vector2Di&          extent) const override {
    return RenderPassPool.GetOrCreate(
        RenderPassVk(colorAttachments, offset, extent));
  }

  virtual jRenderPass* GetOrCreateRenderPass(
      const std::vector<jAttachment>& colorAttachments,
      const jAttachment&              depthAttachment,
      const math::Vector2Di&          offset,
      const math::Vector2Di&          extent) const override {
    return RenderPassPool.GetOrCreate(
        RenderPassVk(colorAttachments, depthAttachment, offset, extent));
  }

  virtual jRenderPass* GetOrCreateRenderPass(
      const std::vector<jAttachment>& colorAttachments,
      const jAttachment&              depthAttachment,
      const jAttachment&              colorResolveAttachment,
      const math::Vector2Di&          offset,
      const math::Vector2Di&          extent) const override {
    return RenderPassPool.GetOrCreate(RenderPassVk(colorAttachments,
                                                   depthAttachment,
                                                   colorResolveAttachment,
                                                   offset,
                                                   extent));
  }

  virtual jRenderPass* GetOrCreateRenderPass(
      const jRenderPassInfo& renderPassInfo,
      const math::Vector2Di& offset,
      const math::Vector2Di& extent) const override {
    return RenderPassPool.GetOrCreate(
        RenderPassVk(renderPassInfo, offset, extent));
  }

  virtual MemoryPoolVk* GetMemoryPool() const override { return MemoryPool; }

  DescriptorPoolVk* GetDescriptorPoolForSingleFrame() const {
    return DescriptorPoolsSingleFrame[CurrentFrameIndex];
  }

  DescriptorPoolVk* GetDescriptorPoolMultiFrame() const {
    return DescriptorPoolMultiFrame;
  }

  RingBufferVk* GetOneFrameUniformRingBuffer() const {
    return OneFrameUniformRingBuffers[CurrentFrameIndex];
  }

  virtual CommandBufferManagerVk* GetCommandBufferManager() const override {
    return CommandBufferManager;
  }

  virtual SemaphoreManagerVk* GetSemaphoreManager() override {
    return &SemaphoreManager;
  }

  virtual FenceManagerVk* GetFenceManager() override { return FenceManager; }

  virtual std::shared_ptr<jSwapchain> GetSwapchain() const override {
    return m_swapchain_;
  }

  virtual uint32_t GetCurrentFrameIndex() const override {
    return CurrentFrameIndex;
  }

  virtual EMSAASamples GetSelectedMSAASamples() const override {
    return SelectedMSAASamples;
  }

  virtual uint32_t GetCurrentFrameNumber() const override {
    return CurrentFrameNumber;
  }

  virtual bool OnHandleResized(uint32_t InWidth,
                               uint32_t InHeight,
                               bool     InIsMinimized) override {
    assert(InWidth > 0);
    assert(InHeight > 0);

    Finish();

    m_swapchain_->Create(m_window_);

    return true;
  }

  virtual void IncrementFrameNumber() { ++CurrentFrameNumber; }

  void DrawArrays(
      const std::shared_ptr<jRenderFrameContext>& InRenderFrameContext,
      /*EPrimitiveType                               type, - deprecated (used in
         previous rendering api)*/
      int32_t                                     vertStartIndex,
      int32_t                                     vertCount) const override;

  void DrawArraysInstanced(
      const std::shared_ptr<jRenderFrameContext>& InRenderFrameContext,
      /*EPrimitiveType                               type, - deprecated (used in
         previous rendering api)*/
      int32_t                                     vertStartIndex,
      int32_t                                     vertCount,
      int32_t                                     instanceCount) const override;

  void DrawElements(
      const std::shared_ptr<jRenderFrameContext>& InRenderFrameContext,
      /*EPrimitiveType                               type, - deprecated (used in
         previous rendering api)*/
      int32_t                                     elementSize,
      int32_t                                     startIndex,
      int32_t                                     indexCount) const override;

  void DrawElementsInstanced(
      const std::shared_ptr<jRenderFrameContext>& InRenderFrameContext,
      /*EPrimitiveType                               type, - deprecated (used in
         previous rendering api)*/
      int32_t                                     elementSize,
      int32_t                                     startIndex,
      int32_t                                     indexCount,
      int32_t                                     instanceCount) const override;

  void DrawElementsBaseVertex(
      const std::shared_ptr<jRenderFrameContext>& InRenderFrameContext,
      /*EPrimitiveType                               type, - deprecated (used in
         previous rendering api)*/
      int32_t                                     elementSize,
      int32_t                                     startIndex,
      int32_t                                     indexCount,
      int32_t baseVertexIndex) const override;

  void DrawElementsInstancedBaseVertex(
      const std::shared_ptr<jRenderFrameContext>& InRenderFrameContext,
      /*EPrimitiveType                               type, - deprecated (used in
         previous rendering api)*/
      int32_t                                     elementSize,
      int32_t                                     startIndex,
      int32_t                                     indexCount,
      int32_t                                     baseVertexIndex,
      int32_t                                     instanceCount) const override;

  void DrawIndirect(
      const std::shared_ptr<jRenderFrameContext>& InRenderFrameContext,
      /*EPrimitiveType                               type, - deprecated (used in
         previous rendering api)*/
      jBuffer*                                    buffer,
      int32_t                                     startIndex,
      int32_t                                     drawCount) const override;

  void DrawElementsIndirect(
      const std::shared_ptr<jRenderFrameContext>& InRenderFrameContext,
      /*EPrimitiveType                               type, - deprecated (used in
         previous rendering api)*/
      jBuffer*                                    buffer,
      int32_t                                     startIndex,
      int32_t                                     drawCount) const override;

  void DispatchCompute(
      const std::shared_ptr<jRenderFrameContext>& InRenderFrameContext,
      uint32_t                                    numGroupsX,
      uint32_t                                    numGroupsY,
      uint32_t                                    numGroupsZ) const override;

  void Flush() const override;

  void Finish() const override;

  void RecreateSwapChain() override;

  virtual std::shared_ptr<jRenderFrameContext> BeginRenderFrame() override;

  virtual void EndRenderFrame(const std::shared_ptr<jRenderFrameContext>&
                                  renderFrameContextPtr) override;

  void QueueSubmit(
      const std::shared_ptr<jRenderFrameContext>& renderFrameContextPtr,
      jSemaphore*                                 InSignalSemaphore) override;

  virtual CommandBufferVk* BeginSingleTimeCommands() const override;

  void EndSingleTimeCommands(jCommandBuffer* commandBuffer) const override;

  virtual void BindGraphicsShaderBindingInstances(
      const jCommandBuffer*                InCommandBuffer,
      const jPipelineStateInfo*            InPiplineState,
      const ShaderBindingInstanceCombiner& InShaderBindingInstanceCombiner,
      std::uint32_t                        InFirstSet) const override;

  // TODO: add methods for VkBufferMemoryBarrier transition layout

  bool TransitionLayout(VkCommandBuffer commandBuffer,
                        VkImage         image,
                        VkFormat        format,
                        uint32_t        mipLevels,
                        uint32_t        layoutCount,
                        VkImageLayout   oldLayout,
                        VkImageLayout   newLayout) const;

  virtual bool TransitionLayout(jCommandBuffer* commandBuffer,
                                jTexture*       texture,
                                EResourceLayout newLayout) const override;

  virtual void BindComputeShaderBindingInstances(
      const jCommandBuffer*                InCommandBuffer,
      const jPipelineStateInfo*            InPiplineState,
      const ShaderBindingInstanceCombiner& InShaderBindingInstanceCombiner,
      std::uint32_t                        InFirstSet) const override;

  // TODO: currently not used
  virtual void NextSubpass(const jCommandBuffer* commandBuffer) const override;

  // TODO: uncomment
  // private:
  const std::vector<const char*> DeviceExtensions
      = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

  void populateDebugMessengerCreateInfo(
      VkDebugUtilsMessengerCreateInfoEXT& createInfo);

  bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface);

  bool CheckDeviceExtensionSupport(VkPhysicalDevice device);

  VkInstance m_instance_;

  VkSurfaceKHR m_surface_;

  VkPhysicalDevice           m_physicalDevice_;
  VkPhysicalDeviceProperties m_deviceProperties_;

  VkDevice m_device_;

  struct QueueVk {
    uint32_t QueueIndex = 0;
    VkQueue  Queue      = nullptr;
  };

  QueueVk m_graphicsQueue_;
  QueueVk m_computeQueue_;
  QueueVk m_presentQueue_;

  VkDebugUtilsMessengerEXT m_debugMessenger_ = nullptr;

  std::shared_ptr<SwapchainVk> m_swapchain_ = std::make_shared<SwapchainVk>();
  // const bool  isVSyncEnabled{true};

  std::shared_ptr<Window>
      m_window_;  // TODO: consider whether need in this place

  CommandBufferManagerVk* CommandBufferManager = nullptr;
  FenceManagerVk*         FenceManager         = nullptr;
  SemaphoreManagerVk      SemaphoreManager;

  VkPipelineCache PipelineCache = nullptr;

  MemoryPoolVk* MemoryPool = nullptr;

  // FrameIndex is swapchain number that is currently used
  uint32_t CurrentFrameIndex = 0;
  // FrameNumber is incremented frame by frame
  uint32_t CurrentFrameNumber = 0;

  std::vector<DescriptorPoolVk*> DescriptorPoolsSingleFrame;
  DescriptorPoolVk*              DescriptorPoolMultiFrame = nullptr;

  std::vector<RingBufferVk*> OneFrameUniformRingBuffers;

  mutable MutexRWLock ShaderBindingPoolLock;

  static std::unordered_map<size_t, ShaderBindingLayoutVk*> ShaderBindingPool;
  static TResourcePool<SamplerStateInfoVk, MutexRWLock>     SamplerStatePool;
  static TResourcePool<RasterizationStateInfoVk, MutexRWLock>
      RasterizationStatePool;
  static TResourcePool<StencilOpStateInfoVk, MutexRWLock> StencilOpStatePool;
  static TResourcePool<DepthStencilStateInfoVk, MutexRWLock>
                                                         DepthStencilStatePool;
  static TResourcePool<BlendingStateInfoVk, MutexRWLock> BlendingStatePool;
  static TResourcePool<PipelineStateInfoVk, MutexRWLock> PipelineStatePool;
  static TResourcePool<RenderPassVk, MutexRWLock>        RenderPassPool;

  bool FramebufferResized = false;

  EMSAASamples SelectedMSAASamples = EMSAASamples::COUNT_1;
};

extern RhiVk* g_rhi_vk;

//// TODO: consider move pipeline state info templated files to another file
//// (current problem - g_rhi_vk dependecty so not possible to move in
//// pipeline_state_info_vk file)
//
///**
// * \brief Constructs a SamplerStateInfoVk object with configurable parameters
// * and a default border color.
// *
// * The \a BorderColor parameter is provided at runtime instead of as a
// * compile-time template parameter due to the complex requirements for
// non-type
// * template parameters (math::Vector4Df does not
// * satisfy the conditions required for a type to be used as a non-type
// template
// * parameter, such as being a literal type with all constexpr constructors).
// *
// */
// template <ETextureFilter         TMinification  = ETextureFilter::NEAREST,
//          ETextureFilter         TMagnification = ETextureFilter::NEAREST,
//          ETextureAddressMode    TAddressU =
//          ETextureAddressMode::CLAMP_TO_EDGE, ETextureAddressMode    TAddressV
//          = ETextureAddressMode::CLAMP_TO_EDGE, ETextureAddressMode TAddressW
//          = ETextureAddressMode::CLAMP_TO_EDGE, float TMipLODBias = 0.0f,
//          float                  TMaxAnisotropy          = 1.0f,
//          bool                   TIsEnableComparisonMode = false,
//          ECompareOp             TComparisonFunc         = ECompareOp::LESS,
//          float                  TMinLOD                 = -FLT_MAX,
//          float                  TMaxLOD                 = FLT_MAX,
//          ETextureComparisonMode TTextureComparisonMode
//          = ETextureComparisonMode::NONE>
// struct TSamplerStateInfoVk {
//  static SamplerStateInfoVk* Create(math::Vector4Df BorderColor
//                                    = math::Vector4Df(0.0f, 0.0f, 0.0f, 1.0f))
//                                    {
//    static SamplerStateInfoVk* CachedInfo = nullptr;
//    if (CachedInfo) {
//      return CachedInfo;
//    }
//
//    SamplerStateInfoVk initializer;
//    initializer.Minification           = TMinification;
//    initializer.Magnification          = TMagnification;
//    initializer.AddressU               = TAddressU;
//    initializer.AddressV               = TAddressV;
//    initializer.AddressW               = TAddressW;
//    initializer.MipLODBias             = TMipLODBias;
//    initializer.MaxAnisotropy          = TMaxAnisotropy;
//    initializer.IsEnableComparisonMode = TIsEnableComparisonMode;
//    initializer.TextureComparisonMode  = TTextureComparisonMode;
//    initializer.ComparisonFunc         = TComparisonFunc;
//    initializer.BorderColor            = BorderColor;
//    initializer.MinLOD                 = TMinLOD;
//    initializer.MaxLOD                 = TMaxLOD;
//    initializer.GetHash();
//    CachedInfo = g_rhi_vk->CreateSamplerState(initializer);
//    return CachedInfo;
//  }
//};
//
// template <EPolygonMode TPolygonMode             = EPolygonMode::FILL,
//          ECullMode    TCullMode                = ECullMode::BACK,
//          EFrontFace   TFrontFace               = EFrontFace::CCW,
//          bool         TDepthBiasEnable         = false,
//          float        TDepthBiasConstantFactor = 0.0f,
//          float        TDepthBiasClamp          = 0.0f,
//          float        TDepthBiasSlopeFactor    = 0.0f,
//          float        TLineWidth               = 1.0f,
//          bool         TDepthClampEnable        = false,
//          bool         TRasterizerDiscardEnable = false,
//          EMSAASamples TSampleCount             = EMSAASamples::COUNT_1,
//          bool         TSampleShadingEnable     = true,
//          float        TMinSampleShading        = 0.2f,
//          bool         TAlphaToCoverageEnable   = false,
//          bool         TAlphaToOneEnable        = false>
// struct TRasterizationStateInfoVk {
//  /**
//   * @brief Creates a rasterization state object based on the template
//   * parameters and/or runtime parameters.
//   *
//   * The template parameters provide default values, while the method
//   parameters
//   * can override them at runtime. If a specific sample count is required
//   * dynamically, pass it as a parameter to the Create method. Otherwise, use
//   * the template parameter TSampleCount for a default value.
//   *
//   * If using the template parameter for sample count, and a dynamic value is
//   * not necessary, pass EMSAASamples::COUNT_1 or any other appropriate
//   * default value as the template argument.
//   */
//  static RasterizationStateInfoVk* Create(
//      std::optional<EMSAASamples> sampleCountOpt = std::nullopt) {
//    static RasterizationStateInfoVk* CachedInfo = nullptr;
//    if (CachedInfo) {
//      return CachedInfo;
//    }
//
//    RasterizationStateInfoVk initializer;
//    initializer.PolygonMode             = TPolygonMode;
//    initializer.CullMode                = TCullMode;
//    initializer.FrontFace               = TFrontFace;
//    initializer.DepthBiasEnable         = TDepthBiasEnable;
//    initializer.DepthBiasConstantFactor = TDepthBiasConstantFactor;
//    initializer.DepthBiasClamp          = TDepthBiasClamp;
//    initializer.DepthBiasSlopeFactor    = TDepthBiasSlopeFactor;
//    initializer.LineWidth               = TLineWidth;
//    initializer.DepthClampEnable        = TDepthClampEnable;
//    initializer.RasterizerDiscardEnable = TRasterizerDiscardEnable;
//
//    initializer.SampleCount           = sampleCountOpt.value_or(TSampleCount);
//    initializer.SampleShadingEnable   = TSampleShadingEnable;
//    initializer.MinSampleShading      = TMinSampleShading;
//    initializer.AlphaToCoverageEnable = TAlphaToCoverageEnable;
//    initializer.AlphaToOneEnable      = TAlphaToOneEnable;
//
//    initializer.GetHash();
//    // TODO: problem (should be in cpp)
//    CachedInfo = g_rhi_vk->CreateRasterizationState(initializer);
//    return CachedInfo;
//  }
//};
//
// template <bool       TDepthTestEnable       = false,
//          bool       TDepthWriteEnable      = false,
//          ECompareOp TDepthCompareOp        = ECompareOp::LEQUAL,
//          bool       TDepthBoundsTestEnable = false,
//          bool       TStencilTestEnable     = false,
//          float      TMinDepthBounds        = 0.0f,
//          float      TMaxDepthBounds        = 1.0f>
// struct TDepthStencilStateInfo {
//  static DepthStencilStateInfoVk* Create(StencilOpStateInfoVk* Front =
//  nullptr,
//                                         StencilOpStateInfoVk* Back = nullptr)
//                                         {
//    static DepthStencilStateInfoVk* CachedInfo = nullptr;
//    if (CachedInfo) {
//      return CachedInfo;
//    }
//
//    DepthStencilStateInfoVk initializer;
//    initializer.DepthTestEnable       = TDepthTestEnable;
//    initializer.DepthWriteEnable      = TDepthWriteEnable;
//    initializer.DepthCompareOp        = TDepthCompareOp;
//    initializer.DepthBoundsTestEnable = TDepthBoundsTestEnable;
//    initializer.StencilTestEnable     = TStencilTestEnable;
//    initializer.Front                 = Front;
//    initializer.Back                  = Back;
//    initializer.MinDepthBounds        = TMinDepthBounds;
//    initializer.MaxDepthBounds        = TMaxDepthBounds;
//    initializer.GetHash();
//    CachedInfo = g_rhi_vk->CreateDepthStencilState(initializer);
//    return CachedInfo;
//  }
//};
//
// template <bool         TBlendEnable    = false,
//          EBlendFactor TSrc            = EBlendFactor::SRC_ALPHA,
//          EBlendFactor TDest           = EBlendFactor::ONE_MINUS_SRC_ALPHA,
//          EBlendOp     TBlendOp        = EBlendOp::ADD,
//          EBlendFactor TSrcAlpha       = EBlendFactor::SRC_ALPHA,
//          EBlendFactor TDestAlpha      = EBlendFactor::ONE_MINUS_SRC_ALPHA,
//          EBlendOp     TAlphaBlendOp   = EBlendOp::ADD,
//          EColorMask   TColorWriteMask = EColorMask::ALL>
// struct TBlendingStateInfo {
//  static BlendingStateInfoVk* Create() {
//    static BlendingStateInfoVk* CachedInfo = nullptr;
//    if (CachedInfo) {
//      return CachedInfo;
//    }
//
//    BlendingStateInfoVk initializer;
//    initializer.BlendEnable    = TBlendEnable;
//    initializer.Src            = TSrc;
//    initializer.Dest           = TDest;
//    initializer.BlendOp        = TBlendOp;
//    initializer.SrcAlpha       = TSrcAlpha;
//    initializer.DestAlpha      = TDestAlpha;
//    initializer.AlphaBlendOp   = TAlphaBlendOp;
//    initializer.ColorWriteMask = TColorWriteMask;
//    initializer.GetHash();
//    CachedInfo = g_rhi_vk->CreateBlendingState(initializer);
//    return CachedInfo;
//  }
//};

}  // namespace game_engine

#endif  // GAME_ENGINE_RHI_VK_H