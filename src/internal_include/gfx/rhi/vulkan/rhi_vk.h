
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
#include "gfx/rhi/vulkan/command_pool_vk.h"
#include "gfx/rhi/vulkan/descriptor_pool_vk.h"
#include "gfx/rhi/vulkan/frame_buffer_pool_vk.h"
#include "gfx/rhi/vulkan/memory_pool_vk.h"
#include "gfx/rhi/vulkan/pipeline_state_info_vk.h"
#include "gfx/rhi/vulkan/render_target_pool_vk.h"
#include "gfx/rhi/vulkan/render_target_vk.h"
#include "gfx/rhi/vulkan/rhi_type_vk.h"
#include "gfx/rhi/vulkan/ring_buffer_vk.h"
#include "gfx/rhi/vulkan/semaphore_vk.h"
#include "gfx/rhi/vulkan/shader_binding_instance_combiner.h"
#include "gfx/rhi/vulkan/swapchain_vk.h"
#include "gfx/rhi/vulkan/uniform_buffer_object_vk.h"
#include "gfx/rhi/vulkan/utils_vk.h"
#include "platform/common/window.h"
#include "utils/logger/global_logger.h"

#include <SDL_vulkan.h>
#include <vulkan/vulkan.h>

#include <set>
#include <vector>

namespace game_engine {

// TODO: move in other place
extern int32_t GMaxCheckCountForRealTimeShaderUpdate;
extern int32_t GSleepMSForRealTimeShaderUpdate;
extern bool    GUseRealTimeShaderUpdate;

extern std::shared_ptr<TextureVk> GWhiteTexture;
extern std::shared_ptr<TextureVk> GBlackTexture;
extern std::shared_ptr<TextureVk> GWhiteCubeTexture;
extern std::shared_ptr<TextureVk> GNormalTexture;
extern std::shared_ptr<Material>  GDefaultMaterial;

class RhiVk {
  public:
  RhiVk();
  RhiVk(const RhiVk&)                    = delete;
  auto operator=(const RhiVk&) -> RhiVk& = delete;
  RhiVk(RhiVk&&)                         = delete;
  auto operator=(RhiVk&&) -> RhiVk&      = delete;
  ~RhiVk()                               = default;

  // TODO: consider change signature for Window object
  bool init(const std::shared_ptr<Window>& window);

  void OnInitRHI();

  void release();

  virtual std::shared_ptr<VertexBufferVk> CreateVertexBuffer(
      const std::shared_ptr<VertexStreamData>& streamData);

  virtual std::shared_ptr<IndexBufferVk> CreateIndexBuffer(
      const std::shared_ptr<IndexStreamData>& streamData) const;

  virtual std::shared_ptr<TextureVk> CreateTextureFromData(
      const ImageData* InImageData) const;

  virtual bool CreateShaderInternal(Shader*           OutShader,
                                    const ShaderInfo& shaderInfo) const;

  FrameBufferVk* CreateFrameBuffer(const FrameBufferInfoVk& info) const;

  std::shared_ptr<RenderTargetVk> CreateRenderTarget(
      const RenderTargetInfoVk& info) const;

  virtual SamplerStateInfoVk* CreateSamplerState(
      const SamplerStateInfoVk& initializer) const {
    return SamplerStatePool.GetOrCreate(initializer);
  }

  virtual RasterizationStateInfoVk* CreateRasterizationState(
      const RasterizationStateInfoVk& initializer) const {
    return RasterizationStatePool.GetOrCreate(initializer);
  }

  virtual StencilOpStateInfoVk* CreateStencilOpStateInfo(
      const StencilOpStateInfoVk& initializer) const {
    return StencilOpStatePool.GetOrCreate(initializer);
  }

  virtual DepthStencilStateInfoVk* CreateDepthStencilState(
      const DepthStencilStateInfoVk& initializer) const {
    return DepthStencilStatePool.GetOrCreate(initializer);
  }

  virtual BlendingStateInfoVk* CreateBlendingState(
      const BlendingStateInfoVk& initializer) const {
    return BlendingStatePool.GetOrCreate(initializer);
  }

  virtual ShaderBindingLayoutVk* CreateShaderBindings(
      const ShaderBindingArray& InShaderBindingArray) const;

  virtual std::shared_ptr<ShaderBindingInstance> CreateShaderBindingInstance(
      const ShaderBindingArray&       InShaderBindingArray,
      const ShaderBindingInstanceType InType) const;

  PipelineStateInfoVk* CreatePipelineStateInfo(
      const PipelineStateFixedInfoVk*   InPipelineStateFixed,
      const GraphicsPipelineShader      InShader,
      const VertexBufferArrayVk&        InVertexBufferArray,
      const RenderPassVk*               InRenderPass,
      const ShaderBindingLayoutArrayVk& InShaderBindingArray,
      const PushConstantVk*             InPushConstant,
      int32_t                           InSubpassIndex) const {
    return PipelineStatePool.GetOrCreateMove(
        std::move(PipelineStateInfoVk(InPipelineStateFixed,
                                      InShader,
                                      InVertexBufferArray,
                                      InRenderPass,
                                      InShaderBindingArray,
                                      InPushConstant,
                                      InSubpassIndex)));
  }

  virtual PipelineStateInfoVk* CreateComputePipelineStateInfo(
      const Shader*                     shader,
      const ShaderBindingLayoutArrayVk& InShaderBindingArray,
      const PushConstantVk*             pushConstant) const {
    return PipelineStatePool.GetOrCreateMove(std::move(
        PipelineStateInfoVk(shader, InShaderBindingArray, pushConstant)));
  }

  // Create Buffers
  std::shared_ptr<BufferVk> CreateBufferInternal(
      uint64_t          InSize,
      uint64_t          InAlignment,
      EBufferCreateFlag InBufferCreateFlag,
      EResourceLayout   InInitialState,
      const void*       InData     = nullptr,
      uint64_t          InDataSize = 0
      /*, const wchar_t*    InResourceName = nullptr*/) const;

  virtual std::shared_ptr<BufferVk> CreateStructuredBuffer(
      uint64_t          InSize,
      uint64_t          InAlignment,
      uint64_t          InStride,
      EBufferCreateFlag InBufferCreateFlag,
      EResourceLayout   InInitialState,
      const void*       InData     = nullptr,
      uint64_t          InDataSize = 0) const {
    return CreateBufferInternal(InSize,
                                InAlignment,
                                InBufferCreateFlag,
                                InInitialState,
                                InData,
                                InDataSize);
  }

  virtual std::shared_ptr<BufferVk> CreateRawBuffer(
      uint64_t          InSize,
      uint64_t          InAlignment,
      EBufferCreateFlag InBufferCreateFlag,
      EResourceLayout   InInitialState,
      const void*       InData     = nullptr,
      uint64_t          InDataSize = 0) const {
    return CreateBufferInternal(InSize,
                                InAlignment,
                                InBufferCreateFlag,
                                InInitialState,
                                InData,
                                InDataSize);
  }

  virtual std::shared_ptr<BufferVk> CreateFormattedBuffer(
      uint64_t          InSize,
      uint64_t          InAlignment,
      VkFormat          InFormat,
      EBufferCreateFlag InBufferCreateFlag,
      EResourceLayout   InInitialState,
      const void*       InData     = nullptr,
      uint64_t          InDataSize = 0) const {
    return CreateBufferInternal(InSize,
                                InAlignment,
                                InBufferCreateFlag,
                                InInitialState,
                                InData,
                                InDataSize);
  }

  virtual std::shared_ptr<IUniformBufferBlock> CreateUniformBufferBlock(
      Name InName, LifeTimeType InLifeTimeType, size_t InSize = 0) const;

  // Create Images
  VkImageUsageFlags GetImageUsageFlags(
      ETextureCreateFlag InTextureCreateFlag) const;

  VkMemoryPropertyFlagBits GetMemoryPropertyFlagBits(
      ETextureCreateFlag InTextureCreateFlag) const;

  virtual std::shared_ptr<TextureVk> Create2DTexture(
      uint32_t              InWidth,
      uint32_t              InHeight,
      uint32_t              InArrayLayers,
      uint32_t              InMipLevels,
      ETextureFormat        InFormat,
      ETextureCreateFlag    InTextureCreateFlag,
      EResourceLayout       InImageLayout   = EResourceLayout::UNDEFINED,
      const ImageBulkData&  InImageBulkData = {},
      const RTClearValueVk& InClearValue    = RTClearValueVk::Invalid,
      const wchar_t*        InResourceName  = nullptr) const;

  virtual std::shared_ptr<TextureVk> CreateCubeTexture(
      uint32_t              InWidth,
      uint32_t              InHeight,
      uint32_t              InMipLevels,
      ETextureFormat        InFormat,
      ETextureCreateFlag    InTextureCreateFlag,
      EResourceLayout       InImageLayout   = EResourceLayout::UNDEFINED,
      const ImageBulkData&  InImageBulkData = {},
      const RTClearValueVk& InClearValue    = RTClearValueVk::Invalid,
      const wchar_t*        InResourceName  = nullptr) const;

  void RemovePipelineStateInfo(size_t InHash) {
    PipelineStatePool.Release(InHash);
  }

  RenderPassVk* GetOrCreateRenderPass(
      const std::vector<AttachmentVk>& colorAttachments,
      const math::Vector2Di&           offset,
      const math::Vector2Di&           extent) const {
    return RenderPassPool.GetOrCreate(
        RenderPassVk(colorAttachments, offset, extent));
  }

  RenderPassVk* GetOrCreateRenderPass(
      const std::vector<AttachmentVk>& colorAttachments,
      const AttachmentVk&              depthAttachment,
      const math::Vector2Di&           offset,
      const math::Vector2Di&           extent) const {
    return RenderPassPool.GetOrCreate(
        RenderPassVk(colorAttachments, depthAttachment, offset, extent));
  }

  RenderPassVk* GetOrCreateRenderPass(
      const std::vector<AttachmentVk>& colorAttachments,
      const AttachmentVk&              depthAttachment,
      const AttachmentVk&              colorResolveAttachment,
      const math::Vector2Di&           offset,
      const math::Vector2Di&           extent) const {
    return RenderPassPool.GetOrCreate(RenderPassVk(colorAttachments,
                                                   depthAttachment,
                                                   colorResolveAttachment,
                                                   offset,
                                                   extent));
  }

  RenderPassVk* GetOrCreateRenderPass(const RenderPassInfoVk& renderPassInfo,
                                      const math::Vector2Di&  offset,
                                      const math::Vector2Di&  extent) const {
    return RenderPassPool.GetOrCreate(
        RenderPassVk(renderPassInfo, offset, extent));
  }

  virtual MemoryPoolVk* GetMemoryPool() const { return MemoryPool; }

  DescriptorPoolVk* GetDescriptorPoolForSingleFrame() const {
    return DescriptorPoolsSingleFrame[CurrentFrameIndex];
  }

  DescriptorPoolVk* GetDescriptorPoolMultiFrame() const {
    return DescriptorPoolMultiFrame;
  }

  RingBufferVk* GetOneFrameUniformRingBuffer() const {
    return OneFrameUniformRingBuffers[CurrentFrameIndex];
  }

  virtual CommandBufferManagerVk* GetCommandBufferManager() const {
    return CommandBufferManager;
  }

  virtual SemaphoreManagerVk* GetSemaphoreManager() {
    return &SemaphoreManager;
  }

  virtual FenceManagerVk* GetFenceManager() { return FenceManager; }

  virtual SwapchainVk GetSwapchain() const { return m_swapchain_; }

  virtual uint32_t GetCurrentFrameIndex() const { return CurrentFrameIndex; }

  virtual EMSAASamples GetSelectedMSAASamples() const {
    return SelectedMSAASamples;
  }

  virtual uint32_t GetCurrentFrameNumber() const { return CurrentFrameNumber; }

  virtual bool OnHandleResized(uint32_t InWidth,
                               uint32_t InHeight,
                               bool     InIsMinimized) {
    assert(InWidth > 0);
    assert(InHeight > 0);

    Finish();

    m_swapchain_.Create(m_window_, m_surface_);

    return true;
  }

  virtual void IncrementFrameNumber() { ++CurrentFrameNumber; }

  void DrawArrays(
      const std::shared_ptr<RenderFrameContextVk>& InRenderFrameContext,
      /*EPrimitiveType                               type, - deprecated (used in
         previous rendering api)*/
      int32_t                                      vertStartIndex,
      int32_t                                      vertCount) const;

  void DrawArraysInstanced(
      const std::shared_ptr<RenderFrameContextVk>& InRenderFrameContext,
      /*EPrimitiveType                               type, - deprecated (used in
         previous rendering api)*/
      int32_t                                      vertStartIndex,
      int32_t                                      vertCount,
      int32_t                                      instanceCount) const;

  void DrawElements(
      const std::shared_ptr<RenderFrameContextVk>& InRenderFrameContext,
      /*EPrimitiveType                               type, - deprecated (used in
         previous rendering api)*/
      int32_t                                      elementSize,
      int32_t                                      startIndex,
      int32_t                                      indexCount) const;

  void DrawElementsInstanced(
      const std::shared_ptr<RenderFrameContextVk>& InRenderFrameContext,
      /*EPrimitiveType                               type, - deprecated (used in
         previous rendering api)*/
      int32_t                                      elementSize,
      int32_t                                      startIndex,
      int32_t                                      indexCount,
      int32_t                                      instanceCount) const;

  void DrawElementsBaseVertex(
      const std::shared_ptr<RenderFrameContextVk>& InRenderFrameContext,
      /*EPrimitiveType                               type, - deprecated (used in
         previous rendering api)*/
      int32_t                                      elementSize,
      int32_t                                      startIndex,
      int32_t                                      indexCount,
      int32_t                                      baseVertexIndex) const;

  void DrawElementsInstancedBaseVertex(
      const std::shared_ptr<RenderFrameContextVk>& InRenderFrameContext,
      /*EPrimitiveType                               type, - deprecated (used in
         previous rendering api)*/
      int32_t                                      elementSize,
      int32_t                                      startIndex,
      int32_t                                      indexCount,
      int32_t                                      baseVertexIndex,
      int32_t                                      instanceCount) const;

  void DrawIndirect(
      const std::shared_ptr<RenderFrameContextVk>& InRenderFrameContext,
      /*EPrimitiveType                               type, - deprecated (used in
         previous rendering api)*/
      BufferVk*                                    buffer,
      int32_t                                      startIndex,
      int32_t                                      drawCount) const;

  void DrawElementsIndirect(
      const std::shared_ptr<RenderFrameContextVk>& InRenderFrameContext,
      /*EPrimitiveType                               type, - deprecated (used in
         previous rendering api)*/
      BufferVk*                                    buffer,
      int32_t                                      startIndex,
      int32_t                                      drawCount) const;

  void DispatchCompute(
      const std::shared_ptr<RenderFrameContextVk>& InRenderFrameContext,
      uint32_t                                     numGroupsX,
      uint32_t                                     numGroupsY,
      uint32_t                                     numGroupsZ) const;

  void Flush() const;

  void Finish() const;

  void RecreateSwapChain();

  virtual std::shared_ptr<RenderFrameContextVk> BeginRenderFrame();

  virtual void EndRenderFrame(
      const std::shared_ptr<RenderFrameContextVk>& renderFrameContextPtr);

  void QueueSubmit(
      const std::shared_ptr<RenderFrameContextVk>& renderFrameContextPtr,
      SemaphoreVk*                                 InSignalSemaphore);

  CommandBufferVk* BeginSingleTimeCommands() const;

  void EndSingleTimeCommands(CommandBufferVk* commandBuffer) const;

  virtual void BindGraphicsShaderBindingInstances(
      const CommandBufferVk*               InCommandBuffer,
      const PipelineStateInfoVk*           InPiplineState,
      const ShaderBindingInstanceCombiner& InShaderBindingInstanceCombiner,
      uint32_t                             InFirstSet) const;

  // TODO: add methods for VkBufferMemoryBarrier transition layout

  bool TransitionLayout(VkCommandBuffer commandBuffer,
                        VkImage         image,
                        VkFormat        format,
                        uint32_t        mipLevels,
                        uint32_t        layoutCount,
                        VkImageLayout   oldLayout,
                        VkImageLayout   newLayout) const;

  virtual bool TransitionLayout(CommandBufferVk* commandBuffer,
                                TextureVk*       texture,
                                EResourceLayout  newLayout) const;

  virtual void BindComputeShaderBindingInstances(
      const CommandBufferVk*               InCommandBuffer,
      const PipelineStateInfoVk*           InPiplineState,
      const ShaderBindingInstanceCombiner& InShaderBindingInstanceCombiner,
      uint32_t                             InFirstSet) const;

  // TODO: currently not used
  virtual void NextSubpass(const CommandBufferVk* commandBuffer) const;

  // BEGIN: shader related functions and variables
  // =================================================================

  static TResourcePool<Shader, MutexRWLock> ShaderPool;

  template <typename T = Shader>
  T* CreateShader(const ShaderInfo& InShaderInfo) const {
    return (T*)ShaderPool.GetOrCreate<ShaderInfo, T>(InShaderInfo);
  }

  void AddShader(const ShaderInfo& InShaderInfo, Shader* InShader) {
    return ShaderPool.Add(InShaderInfo, InShader);
  }

  void ReleaseShader(const ShaderInfo& InShaderInfo) {
    ShaderPool.Release(InShaderInfo);
  }

  std::vector<Shader*> GetAllShaders() {
    std::vector<Shader*> Out;
    ShaderPool.GetAllResource(Out);
    return Out;
  }

  // END: shader related functions and variables
  // =================================================================

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

  SwapchainVk m_swapchain_;
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

// TODO: consider move pipeline state info templated files to another file
// (current problem - g_rhi_vk dependecty so not possible to move in
// pipeline_state_info_vk file)

/**
 * \brief Constructs a SamplerStateInfoVk object with configurable parameters
 * and a default border color.
 *
 * The \a BorderColor parameter is provided at runtime instead of as a
 * compile-time template parameter due to the complex requirements for non-type
 * template parameters (math::Vector4Df does not
 * satisfy the conditions required for a type to be used as a non-type template
 * parameter, such as being a literal type with all constexpr constructors).
 *
 */
template <ETextureFilter         TMinification  = ETextureFilter::NEAREST,
          ETextureFilter         TMagnification = ETextureFilter::NEAREST,
          ETextureAddressMode    TAddressU = ETextureAddressMode::CLAMP_TO_EDGE,
          ETextureAddressMode    TAddressV = ETextureAddressMode::CLAMP_TO_EDGE,
          ETextureAddressMode    TAddressW = ETextureAddressMode::CLAMP_TO_EDGE,
          float                  TMipLODBias             = 0.0f,
          float                  TMaxAnisotropy          = 1.0f,
          bool                   TIsEnableComparisonMode = false,
          ECompareOp             TComparisonFunc         = ECompareOp::LESS,
          float                  TMinLOD                 = -FLT_MAX,
          float                  TMaxLOD                 = FLT_MAX,
          ETextureComparisonMode TTextureComparisonMode
          = ETextureComparisonMode::NONE>
struct TSamplerStateInfoVk {
  static SamplerStateInfoVk* Create(math::Vector4Df BorderColor
                                    = math::Vector4Df(0.0f, 0.0f, 0.0f, 1.0f)) {
    static SamplerStateInfoVk* CachedInfo = nullptr;
    if (CachedInfo) {
      return CachedInfo;
    }

    SamplerStateInfoVk initializer;
    initializer.Minification           = TMinification;
    initializer.Magnification          = TMagnification;
    initializer.AddressU               = TAddressU;
    initializer.AddressV               = TAddressV;
    initializer.AddressW               = TAddressW;
    initializer.MipLODBias             = TMipLODBias;
    initializer.MaxAnisotropy          = TMaxAnisotropy;
    initializer.IsEnableComparisonMode = TIsEnableComparisonMode;
    initializer.TextureComparisonMode  = TTextureComparisonMode;
    initializer.ComparisonFunc         = TComparisonFunc;
    initializer.BorderColor            = BorderColor;
    initializer.MinLOD                 = TMinLOD;
    initializer.MaxLOD                 = TMaxLOD;
    initializer.GetHash();
    CachedInfo = g_rhi_vk->CreateSamplerState(initializer);
    return CachedInfo;
  }
};

template <EPolygonMode TPolygonMode             = EPolygonMode::FILL,
          ECullMode    TCullMode                = ECullMode::BACK,
          EFrontFace   TFrontFace               = EFrontFace::CCW,
          bool         TDepthBiasEnable         = false,
          float        TDepthBiasConstantFactor = 0.0f,
          float        TDepthBiasClamp          = 0.0f,
          float        TDepthBiasSlopeFactor    = 0.0f,
          float        TLineWidth               = 1.0f,
          bool         TDepthClampEnable        = false,
          bool         TRasterizerDiscardEnable = false,
          EMSAASamples TSampleCount             = EMSAASamples::COUNT_1,
          bool         TSampleShadingEnable     = true,
          float        TMinSampleShading        = 0.2f,
          bool         TAlphaToCoverageEnable   = false,
          bool         TAlphaToOneEnable        = false>
struct TRasterizationStateInfoVk {
  /**
   * @brief Creates a rasterization state object based on the template
   * parameters and/or runtime parameters.
   *
   * The template parameters provide default values, while the method parameters
   * can override them at runtime. If a specific sample count is required
   * dynamically, pass it as a parameter to the Create method. Otherwise, use
   * the template parameter TSampleCount for a default value.
   *
   * If using the template parameter for sample count, and a dynamic value is
   * not necessary, pass EMSAASamples::COUNT_1 or any other appropriate
   * default value as the template argument.
   */
  static RasterizationStateInfoVk* Create(
      std::optional<EMSAASamples> sampleCountOpt = std::nullopt) {
    static RasterizationStateInfoVk* CachedInfo = nullptr;
    if (CachedInfo) {
      return CachedInfo;
    }

    RasterizationStateInfoVk initializer;
    initializer.PolygonMode             = TPolygonMode;
    initializer.CullMode                = TCullMode;
    initializer.FrontFace               = TFrontFace;
    initializer.DepthBiasEnable         = TDepthBiasEnable;
    initializer.DepthBiasConstantFactor = TDepthBiasConstantFactor;
    initializer.DepthBiasClamp          = TDepthBiasClamp;
    initializer.DepthBiasSlopeFactor    = TDepthBiasSlopeFactor;
    initializer.LineWidth               = TLineWidth;
    initializer.DepthClampEnable        = TDepthClampEnable;
    initializer.RasterizerDiscardEnable = TRasterizerDiscardEnable;

    initializer.SampleCount           = sampleCountOpt.value_or(TSampleCount);
    initializer.SampleShadingEnable   = TSampleShadingEnable;
    initializer.MinSampleShading      = TMinSampleShading;
    initializer.AlphaToCoverageEnable = TAlphaToCoverageEnable;
    initializer.AlphaToOneEnable      = TAlphaToOneEnable;

    initializer.GetHash();
    // TODO: problem (should be in cpp)
    CachedInfo = g_rhi_vk->CreateRasterizationState(initializer);
    return CachedInfo;
  }
};

template <bool       TDepthTestEnable       = false,
          bool       TDepthWriteEnable      = false,
          ECompareOp TDepthCompareOp        = ECompareOp::LEQUAL,
          bool       TDepthBoundsTestEnable = false,
          bool       TStencilTestEnable     = false,
          float      TMinDepthBounds        = 0.0f,
          float      TMaxDepthBounds        = 1.0f>
struct TDepthStencilStateInfo {
  static DepthStencilStateInfoVk* Create(StencilOpStateInfoVk* Front = nullptr,
                                         StencilOpStateInfoVk* Back = nullptr) {
    static DepthStencilStateInfoVk* CachedInfo = nullptr;
    if (CachedInfo) {
      return CachedInfo;
    }

    DepthStencilStateInfoVk initializer;
    initializer.DepthTestEnable       = TDepthTestEnable;
    initializer.DepthWriteEnable      = TDepthWriteEnable;
    initializer.DepthCompareOp        = TDepthCompareOp;
    initializer.DepthBoundsTestEnable = TDepthBoundsTestEnable;
    initializer.StencilTestEnable     = TStencilTestEnable;
    initializer.Front                 = Front;
    initializer.Back                  = Back;
    initializer.MinDepthBounds        = TMinDepthBounds;
    initializer.MaxDepthBounds        = TMaxDepthBounds;
    initializer.GetHash();
    CachedInfo = g_rhi_vk->CreateDepthStencilState(initializer);
    return CachedInfo;
  }
};

template <bool         TBlendEnable    = false,
          EBlendFactor TSrc            = EBlendFactor::SRC_ALPHA,
          EBlendFactor TDest           = EBlendFactor::ONE_MINUS_SRC_ALPHA,
          EBlendOp     TBlendOp        = EBlendOp::ADD,
          EBlendFactor TSrcAlpha       = EBlendFactor::SRC_ALPHA,
          EBlendFactor TDestAlpha      = EBlendFactor::ONE_MINUS_SRC_ALPHA,
          EBlendOp     TAlphaBlendOp   = EBlendOp::ADD,
          EColorMask   TColorWriteMask = EColorMask::ALL>
struct TBlendingStateInfo {
  static BlendingStateInfoVk* Create() {
    static BlendingStateInfoVk* CachedInfo = nullptr;
    if (CachedInfo) {
      return CachedInfo;
    }

    BlendingStateInfoVk initializer;
    initializer.BlendEnable    = TBlendEnable;
    initializer.Src            = TSrc;
    initializer.Dest           = TDest;
    initializer.BlendOp        = TBlendOp;
    initializer.SrcAlpha       = TSrcAlpha;
    initializer.DestAlpha      = TDestAlpha;
    initializer.AlphaBlendOp   = TAlphaBlendOp;
    initializer.ColorWriteMask = TColorWriteMask;
    initializer.GetHash();
    CachedInfo = g_rhi_vk->CreateBlendingState(initializer);
    return CachedInfo;
  }
};

}  // namespace game_engine

#endif