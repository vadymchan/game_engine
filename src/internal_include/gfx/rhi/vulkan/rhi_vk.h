
// TODO: add utils class for functions and variables

#ifndef GAME_ENGINE_RHI_VK_H
#define GAME_ENGINE_RHI_VK_H

#ifndef NDEBUG  // DEBUG
#define ENABLE_VALIDATION_LAYERS
#endif

#define VALIDATION_LAYER_VERBOSE 0

#include "file_loader/file.h"
#include "file_loader/image_file_loader.h"
#include "gfx/rhi/lock.h"
#include "gfx/rhi/resource_pool.h"
#include "gfx/rhi/rhi.h"
#include "gfx/rhi/vulkan/command_pool_vk.h"
#include "gfx/rhi/vulkan/descriptor_pool_vk.h"
#include "gfx/rhi/vulkan/memory_pool_vk.h"
#include "gfx/rhi/vulkan/pipeline_state_info_vk.h"
#include "gfx/rhi/vulkan/render_frame_context_vk.h"
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

// TODO:
// - UAVBarrier / UAVBarrierImmediate
// - TransitionLayoutImmediate
// - GetSwapchainImage
// - etc. find fro general rhi

class RhiVk : public RHI {
  public:
  // ======= BEGIN: public nested types =======================================

  struct QueueVk {
    uint32_t m_queueIndex_ = 0;
    VkQueue  m_queue_      = nullptr;
  };

  // ======= END: public nested types   =======================================

  // ======= BEGIN: public static fields ======================================

  static std::unordered_map<size_t, std::shared_ptr<ShaderBindingLayoutVk>>
                                                        s_shaderBindingPool;
  static TResourcePool<SamplerStateInfoVk, MutexRWLock> s_samplerStatePool;
  static TResourcePool<RasterizationStateInfoVk, MutexRWLock>
      s_rasterizationStatePool;
  static TResourcePool<StencilOpStateInfoVk, MutexRWLock> s_stencilOpStatePool;
  static TResourcePool<DepthStencilStateInfoVk, MutexRWLock>
      s_depthStencilStatePool;
  static TResourcePool<BlendingStateInfoVk, MutexRWLock> s_blendingStatePool;
  static TResourcePool<PipelineStateInfoVk, MutexRWLock> s_pipelineStatePool;
  static TResourcePool<RenderPassVk, MutexRWLock>        s_renderPassPool;

  // ======= END: public static fields   ======================================

  // ======= BEGIN: public constructors =======================================

  RhiVk();
  RhiVk(const RhiVk&) = delete;
  RhiVk(RhiVk&&)      = delete;

  // ======= END: public constructors   =======================================

  // ======= BEGIN: public destructor =========================================

  virtual ~RhiVk() = default;

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overridden methods =================================

  // TODO: consider group and sort methods

  // TODO: consider change signature for Window object
  virtual bool init(const std::shared_ptr<Window>& window) override;

  virtual void release() override;

  virtual std::shared_ptr<VertexBuffer> createVertexBuffer(
      const std::shared_ptr<VertexStreamData>& streamData) const override;

  virtual std::shared_ptr<IndexBuffer> createIndexBuffer(
      const std::shared_ptr<IndexStreamData>& streamData) const override;

  virtual std::shared_ptr<Texture> createTextureFromData(
      const std::shared_ptr<Image>& image) const override;

  virtual bool createShaderInternal(
      Shader* shader, const ShaderInfo& shaderInfo) const override;

  virtual FrameBuffer* createFrameBuffer(
      const FrameBufferInfo& info) const override;

  virtual std::shared_ptr<RenderTarget> createRenderTarget(
      const RenderTargetInfo& info) const override;

  virtual SamplerStateInfo* createSamplerState(
      const SamplerStateInfo& initializer) const override {
    return s_samplerStatePool.getOrCreate(initializer);
  }

  virtual RasterizationStateInfo* createRasterizationState(
      const RasterizationStateInfo& initializer) const override {
    return s_rasterizationStatePool.getOrCreate(initializer);
  }

  virtual StencilOpStateInfo* createStencilOpStateInfo(
      const StencilOpStateInfo& initializer) const override {
    return s_stencilOpStatePool.getOrCreate(initializer);
  }

  virtual DepthStencilStateInfo* createDepthStencilState(
      const DepthStencilStateInfo& initializer) const override {
    return s_depthStencilStatePool.getOrCreate(initializer);
  }

  virtual BlendingStateInfo* createBlendingState(
      const BlendingStateInfo& initializer) const override {
    return s_blendingStatePool.getOrCreate(initializer);
  }

  virtual std::shared_ptr<ShaderBindingLayout> createShaderBindings(
      const ShaderBindingArray& shaderBindingArray) const override;

  virtual std::shared_ptr<ShaderBindingInstance> createShaderBindingInstance(
      const ShaderBindingArray&       shaderBindingArray,
      const ShaderBindingInstanceType type) override;

  virtual PipelineStateInfo* createPipelineStateInfo(
      const PipelineStateFixedInfo*   pipelineStateFixed,
      const GraphicsPipelineShader    shader,
      const VertexBufferArray&        vertexBufferArray,
      const RenderPass*               renderPass,
      const ShaderBindingLayoutArray& shaderBindingArray,
      const PushConstant*             pushConstant,
      std::int32_t                    subpassIndex) const override {
    return s_pipelineStatePool.getOrCreateMove(
        std::move(PipelineStateInfo(pipelineStateFixed,
                                    shader,
                                    vertexBufferArray,
                                    renderPass,
                                    shaderBindingArray,
                                    pushConstant,
                                    subpassIndex)));
  }

  virtual PipelineStateInfo* createComputePipelineStateInfo(
      const Shader*                   shader,
      const ShaderBindingLayoutArray& shaderBindingArray,
      const PushConstant*             pushConstant) const override {
    return s_pipelineStatePool.getOrCreateMove(
        std::move(PipelineStateInfo(shader, shaderBindingArray, pushConstant)));
  }

  // Create Buffers
  virtual std::shared_ptr<IBuffer> createStructuredBuffer(
      std::uint64_t     size,
      std::uint64_t     alignment,
      std::uint64_t     stride,
      EBufferCreateFlag bufferCreateFlag,
      EResourceLayout   initialState,
      const void*       data     = nullptr,
      std::uint64_t     dataSize = 0) const override {
    return createBufferInternal(
        size, alignment, bufferCreateFlag, initialState, data, dataSize);
  }

  virtual std::shared_ptr<IBuffer> createRawBuffer(
      std::uint64_t     size,
      std::uint64_t     alignment,
      EBufferCreateFlag bufferCreateFlag,
      EResourceLayout   initialState,
      const void*       data     = nullptr,
      std::uint64_t     dataSize = 0) const override {
    return createBufferInternal(
        size, alignment, bufferCreateFlag, initialState, data, dataSize);
  }

  virtual std::shared_ptr<IBuffer> createFormattedBuffer(
      std::uint64_t     size,
      std::uint64_t     alignment,
      ETextureFormat    format,
      EBufferCreateFlag bufferCreateFlag,
      EResourceLayout   initialState,
      const void*       data     = nullptr,
      std::uint64_t     dataSize = 0) const override {
    return createBufferInternal(
        size, alignment, bufferCreateFlag, initialState, data, dataSize);
  }

  virtual std::shared_ptr<IUniformBufferBlock> createUniformBufferBlock(
      Name name, LifeTimeType lifeTimeType, size_t size = 0) const override;

  // Create Images
  virtual std::shared_ptr<Texture> create2DTexture(
      uint32_t                      width,
      uint32_t                      height,
      uint32_t                      arrayLayers,
      uint32_t                      mipLevels,
      ETextureFormat                format,
      ETextureCreateFlag            textureCreateFlag,
      EResourceLayout               imageLayout  = EResourceLayout::UNDEFINED,
      const std::shared_ptr<Image>& image        = nullptr,
      const RtClearValue&           clearValue   = RtClearValue::s_kInvalid,
      const wchar_t*                resourceName = nullptr) const override;

  virtual bool onHandleResized(uint32_t witdh,
                               uint32_t height,
                               bool     isMinimized) override {
    assert(witdh > 0);
    assert(height > 0);

    finish();

    m_swapchain_->create(m_window_);

    return true;
  }

  virtual void incrementFrameNumber() override { ++m_currentFrameNumber_; }

  void drawArrays(const std::shared_ptr<CommandBuffer>& commandBuffer,
                  int32_t                               vertStartIndex,
                  int32_t vertCount) const override;

  void drawArraysInstanced(const std::shared_ptr<CommandBuffer>& commandBuffer,
                           int32_t                               vertStartIndex,
                           int32_t                               vertCount,
                           int32_t instanceCount) const override;

  void drawElements(const std::shared_ptr<CommandBuffer>& commandBuffer,
                    int32_t                               startIndex,
                    int32_t indexCount) const override;

  void drawElementsInstanced(
      const std::shared_ptr<CommandBuffer>& commandBuffer,
      int32_t                               startIndex,
      int32_t                               indexCount,
      int32_t                               instanceCount) const override;

  void drawElementsBaseVertex(
      const std::shared_ptr<CommandBuffer>& commandBuffer,
      int32_t                               startIndex,
      int32_t                               indexCount,
      int32_t                               baseVertexIndex) const override;

  void drawElementsInstancedBaseVertex(
      const std::shared_ptr<CommandBuffer>& commandBuffer,
      int32_t                               startIndex,
      int32_t                               indexCount,
      int32_t                               baseVertexIndex,
      int32_t                               instanceCount) const override;

  void drawIndirect(const std::shared_ptr<CommandBuffer>& commandBuffer,
                    IBuffer*                              buffer,
                    int32_t                               startIndex,
                    int32_t drawCount) const override;

  void drawElementsIndirect(const std::shared_ptr<CommandBuffer>& commandBuffer,
                            IBuffer*                              buffer,
                            int32_t                               startIndex,
                            int32_t drawCount) const override;

  void dispatchCompute(const std::shared_ptr<CommandBuffer>& commandBuffer,
                       uint32_t                              numGroupsX,
                       uint32_t                              numGroupsY,
                       uint32_t numGroupsZ) const override;

  void flush() const override;

  void finish() const override;

  void recreateSwapChain() override;

  void queueSubmit(
      const std::shared_ptr<RenderFrameContext>& renderFrameContextPtr,
      ISemaphore*                                signalSemaphore) override;

  virtual std::shared_ptr<RenderFrameContext> beginRenderFrame() override;

  virtual void endRenderFrame(const std::shared_ptr<RenderFrameContext>&
                                  renderFrameContextPtr) override;

  virtual std::shared_ptr<CommandBuffer> beginSingleTimeCommands()
      const override;

  void endSingleTimeCommands(
      std::shared_ptr<CommandBuffer> commandBuffer) const override;

  virtual bool transitionLayout(std::shared_ptr<CommandBuffer> commandBuffer,
                                Texture*                       texture,
                                EResourceLayout newLayout) const override;

  virtual void bindGraphicsShaderBindingInstances(
      const std::shared_ptr<CommandBuffer> commandBuffer,
      const PipelineStateInfo*             piplineState,
      const ShaderBindingInstanceCombiner& shaderBindingInstanceCombiner,
      std::uint32_t                        firstSet) const override;

  virtual void bindComputeShaderBindingInstances(
      const std::shared_ptr<CommandBuffer> commandBuffer,
      const PipelineStateInfo*             piplineState,
      const ShaderBindingInstanceCombiner& shaderBindingInstanceCombiner,
      std::uint32_t                        firstSet) const override;

  // TODO: currently not used
  virtual void nextSubpass(
      const std::shared_ptr<CommandBuffer> commandBuffer) const override;

  virtual RenderPass* getOrCreateRenderPass(
      const std::vector<Attachment>& colorAttachments,
      const math::Vector2Di&         offset,
      const math::Vector2Di&         extent) const override {
    return s_renderPassPool.getOrCreate(
        RenderPassVk(colorAttachments, offset, extent));
  }

  virtual RenderPass* getOrCreateRenderPass(
      const std::vector<Attachment>& colorAttachments,
      const Attachment&              depthAttachment,
      const math::Vector2Di&         offset,
      const math::Vector2Di&         extent) const override {
    return s_renderPassPool.getOrCreate(
        RenderPassVk(colorAttachments, depthAttachment, offset, extent));
  }

  virtual RenderPass* getOrCreateRenderPass(
      const std::vector<Attachment>& colorAttachments,
      const Attachment&              depthAttachment,
      const Attachment&              colorResolveAttachment,
      const math::Vector2Di&         offset,
      const math::Vector2Di&         extent) const override {
    return s_renderPassPool.getOrCreate(RenderPassVk(colorAttachments,
                                                     depthAttachment,
                                                     colorResolveAttachment,
                                                     offset,
                                                     extent));
  }

  virtual RenderPass* getOrCreateRenderPass(
      const RenderPassInfo&  renderPassInfo,
      const math::Vector2Di& offset,
      const math::Vector2Di& extent) const override {
    return s_renderPassPool.getOrCreate(
        RenderPassVk(renderPassInfo, offset, extent));
  }

  virtual MemoryPoolVk* getMemoryPool() const override { return m_memoryPool_; }

  virtual CommandBufferManagerVk* getCommandBufferManager() const override {
    return m_commandBufferManager_;
  }

  virtual SemaphoreManagerVk* getSemaphoreManager() override {
    return &m_semaphoreManager_;
  }

  virtual FenceManagerVk* getFenceManager() override { return m_fenceManager_; }

  virtual std::shared_ptr<ISwapchain> getSwapchain() const override {
    return m_swapchain_;
  }

  virtual uint32_t getCurrentFrameIndex() const override {
    return m_currentFrameIndex_;
  }

  virtual EMSAASamples getSelectedMSAASamples() const override {
    return m_selectedMSAASamples_;
  }

  virtual uint32_t getCurrentFrameNumber() const override {
    return m_currentFrameNumber_;
  }

  // ======= END: public overridden methods   =================================

  // ======= BEGIN: public getters ============================================

  DescriptorPoolVk* getDescriptorPoolForSingleFrame() const {
    return m_descriptorPoolsSingleFrame_[m_currentFrameIndex_];
  }

  DescriptorPoolVk* getDescriptorPoolMultiFrame() const {
    return m_descriptorPoolMultiFrame_;
  }

  RingBufferVk* getOneFrameUniformRingBuffer() const {
    return m_oneFrameUniformRingBuffers_[m_currentFrameIndex_];
  }

  // Create Images
  VkImageUsageFlags getImageUsageFlags(
      ETextureCreateFlag textureCreateFlag) const;

  VkMemoryPropertyFlagBits getMemoryPropertyFlagBits(
      ETextureCreateFlag textureCreateFlag) const;

  // ======= END: public getters   ============================================

  // ======= BEGIN: public misc methods =======================================

  void removePipelineStateInfo(size_t hash) {
    s_pipelineStatePool.release(hash);
  }

  // Create Buffers
  std::shared_ptr<IBuffer> createBufferInternal(
      std::uint64_t     size,
      std::uint64_t     alignment,
      EBufferCreateFlag bufferCreateFlag,
      EResourceLayout   initialState,
      const void*       data     = nullptr,
      std::uint64_t     dataSize = 0
      /*, const wchar_t*    resourceName = nullptr*/) const;

  // TODO: add methods for VkBufferMemoryBarrier transition layout
  bool transitionLayout(VkCommandBuffer commandBuffer,
                        VkImage         image,
                        VkFormat        format,
                        uint32_t        mipLevels,
                        uint32_t        layoutCount,
                        VkImageLayout   oldLayout,
                        VkImageLayout   newLayout) const;

  void populateDebugMessengerCreateInfo(
      VkDebugUtilsMessengerCreateInfoEXT& createInfo);

  bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface);

  bool checkDeviceExtensionSupport(VkPhysicalDevice device);

  // ======= END: public misc methods   =======================================

  // ======= BEGIN: public overloaded operators ===============================

  auto operator=(const RhiVk&) -> RhiVk& = delete;
  auto operator=(RhiVk&&) -> RhiVk&      = delete;

  // ======= END: public overloaded operators   ===============================

  // ======= BEGIN: public constants ==========================================

  const std::vector<const char*> kDeviceExtensions
      = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
  // const bool  m_isVSyncEnabled_{true};

  // ======= END: public constants   ==========================================

  // ======= BEGIN: public misc fields ========================================

  VkInstance m_instance_;

  VkSurfaceKHR m_surface_;

  VkPhysicalDevice           m_physicalDevice_;
  VkPhysicalDeviceProperties m_deviceProperties_;

  VkDevice m_device_;

  QueueVk m_graphicsQueue_;
  QueueVk m_computeQueue_;
  QueueVk m_presentQueue_;

  VkDebugUtilsMessengerEXT m_debugMessenger_ = nullptr;

  std::shared_ptr<SwapchainVk> m_swapchain_ = std::make_shared<SwapchainVk>();

  // TODO: consider whether need in this place
  std::shared_ptr<Window> m_window_;

  CommandBufferManagerVk* m_commandBufferManager_ = nullptr;
  FenceManagerVk*         m_fenceManager_         = nullptr;
  SemaphoreManagerVk      m_semaphoreManager_;

  VkPipelineCache m_pipelineCache_ = nullptr;

  MemoryPoolVk* m_memoryPool_ = nullptr;

  // FrameIndex is swapchain number that is currently used
  uint32_t m_currentFrameIndex_ = 0;
  // FrameNumber is incremented frame by frame
  uint32_t m_currentFrameNumber_ = 0;

  std::vector<DescriptorPoolVk*> m_descriptorPoolsSingleFrame_;
  DescriptorPoolVk*              m_descriptorPoolMultiFrame_ = nullptr;

  std::vector<RingBufferVk*> m_oneFrameUniformRingBuffers_;

  mutable MutexRWLock m_shaderBindingPoolLock_;

  bool m_isFramebufferResized_ = false;

  EMSAASamples m_selectedMSAASamples_ = EMSAASamples::COUNT_1;

  // ======= END: public misc fields   ========================================
};

extern RhiVk* g_rhiVk;

}  // namespace game_engine

#endif  // GAME_ENGINE_RHI_VK_H
