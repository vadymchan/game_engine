
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

// TODO:
// - UAVBarrier / UAVBarrierImmediate
// - TransitionLayoutImmediate
// - GetSwapchainImage
// - etc. find fro general rhi

class RhiVk : public RHI {
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

  virtual std::shared_ptr<VertexBuffer> CreateVertexBuffer(
      const std::shared_ptr<VertexStreamData>& streamData) const override;

  virtual std::shared_ptr<IndexBuffer> CreateIndexBuffer(
      const std::shared_ptr<IndexStreamData>& streamData) const override;

  virtual std::shared_ptr<Texture> CreateTextureFromData(
      const ImageData* imageData) const override;

  virtual bool CreateShaderInternal(
      Shader* OutShader, const ShaderInfo& shaderInfo) const override;

  virtual FrameBuffer* CreateFrameBuffer(
      const FrameBufferInfo& info) const override;

  virtual std::shared_ptr<RenderTarget> CreateRenderTarget(
      const RenderTargetInfo& info) const override;

  virtual SamplerStateInfo* CreateSamplerState(
      const SamplerStateInfo& initializer) const override {
    return s_samplerStatePool.GetOrCreate(initializer);
  }

  virtual RasterizationStateInfo* CreateRasterizationState(
      const RasterizationStateInfo& initializer) const override {
    return s_rasterizationStatePool.GetOrCreate(initializer);
  }

  virtual StencilOpStateInfo* CreateStencilOpStateInfo(
      const StencilOpStateInfo& initializer) const override {
    return s_stencilOpStatePool.GetOrCreate(initializer);
  }

  virtual DepthStencilStateInfo* CreateDepthStencilState(
      const DepthStencilStateInfo& initializer) const override {
    return s_depthStencilStatePool.GetOrCreate(initializer);
  }

  virtual BlendingStateInfo* CreateBlendingState(
      const BlendingStateInfo& initializer) const override {
    return s_blendingStatePool.GetOrCreate(initializer);
  }

  virtual ShaderBindingLayout* CreateShaderBindings(
      const ShaderBindingArray& shaderBindingArray) const override;

  virtual std::shared_ptr<ShaderBindingInstance> CreateShaderBindingInstance(
      const ShaderBindingArray&      shaderBindingArray,
      const ShaderBindingInstanceType type) const override;

  virtual PipelineStateInfo* CreatePipelineStateInfo(
      const PipelineStateFixedInfo*   pipelineStateFixed,
      const GraphicsPipelineShader     shader,
      const VertexBufferArray&         vertexBufferArray,
      const RenderPass*                renderPass,
      const ShaderBindingLayoutArray& shaderBindingArray,
      const PushConstant*             pushConstant,
      std::int32_t                     subpassIndex) const override {
    return s_pipelineStatePool.GetOrCreateMove(
        std::move(PipelineStateInfo(pipelineStateFixed,
                                     shader,
                                     vertexBufferArray,
                                     renderPass,
                                     shaderBindingArray,
                                     pushConstant,
                                     subpassIndex)));
  }

  virtual PipelineStateInfo* CreateComputePipelineStateInfo(
      const Shader*                    shader,
      const ShaderBindingLayoutArray& shaderBindingArray,
      const PushConstant*             pushConstant) const override {
    return s_pipelineStatePool.GetOrCreateMove(std::move(
        PipelineStateInfo(shader, shaderBindingArray, pushConstant)));
  }

  // Create m_buffers
  std::shared_ptr<Buffer> CreateBufferInternal(
      std::uint64_t     size,
      std::uint64_t     alignment,
      EBufferCreateFlag bufferCreateFlag,
      EResourceLayout   initialState,
      const void*       data     = nullptr,
      std::uint64_t     dataSize = 0
      /*, const wchar_t*    resourceName = nullptr*/) const;

  virtual std::shared_ptr<Buffer> CreateStructuredBuffer(
      std::uint64_t     size,
      std::uint64_t     alignment,
      std::uint64_t     stride,
      EBufferCreateFlag bufferCreateFlag,
      EResourceLayout   initialState,
      const void*       data     = nullptr,
      std::uint64_t     dataSize = 0) const override {
    return CreateBufferInternal(size,
                                alignment,
                                bufferCreateFlag,
                                initialState,
                                data,
                                dataSize);
  }

  virtual std::shared_ptr<Buffer> CreateRawBuffer(
      std::uint64_t     size,
      std::uint64_t     alignment,
      EBufferCreateFlag bufferCreateFlag,
      EResourceLayout   initialState,
      const void*       data     = nullptr,
      std::uint64_t     dataSize = 0) const override {
    return CreateBufferInternal(size,
                                alignment,
                                bufferCreateFlag,
                                initialState,
                                data,
                                dataSize);
  }

  virtual std::shared_ptr<Buffer> CreateFormattedBuffer(
      std::uint64_t     size,
      std::uint64_t     alignment,
      ETextureFormat    format,
      EBufferCreateFlag bufferCreateFlag,
      EResourceLayout   initialState,
      const void*       data     = nullptr,
      std::uint64_t     dataSize = 0) const override {
    return CreateBufferInternal(size,
                                alignment,
                                bufferCreateFlag,
                                initialState,
                                data,
                                dataSize);
  }

  virtual std::shared_ptr<IUniformBufferBlock> CreateUniformBufferBlock(
      Name         name,
      LifeTimeType lifeTimeType,
      size_t       size = 0) const override;

  // Create Images
  VkImageUsageFlags GetImageUsageFlags(
      ETextureCreateFlag InTextureCreateFlag) const;

  VkMemoryPropertyFlagBits GetMemoryPropertyFlagBits(
      ETextureCreateFlag InTextureCreateFlag) const;

  virtual std::shared_ptr<Texture> Create2DTexture(
      uint32_t             witdh,
      uint32_t             height,
      uint32_t             InArrayLayers,
      uint32_t             InMipLevels,
      ETextureFormat       format,
      ETextureCreateFlag   InTextureCreateFlag,
      EResourceLayout      InImageLayout   = EResourceLayout::UNDEFINED,
      const ImageBulkData& InImageBulkData = {},
      const RTClearValue&  InClearValue    = RTClearValue::s_kInvalid,
      const wchar_t*       resourceName  = nullptr) const override;

  virtual std::shared_ptr<Texture> CreateCubeTexture(
      uint32_t             witdh,
      uint32_t             height,
      uint32_t             InMipLevels,
      ETextureFormat       format,
      ETextureCreateFlag   InTextureCreateFlag,
      EResourceLayout      InImageLayout   = EResourceLayout::UNDEFINED,
      const ImageBulkData& InImageBulkData = {},
      const RTClearValue&  InClearValue    = RTClearValue::s_kInvalid,
      const wchar_t*       resourceName  = nullptr) const override;

  void RemovePipelineStateInfo(size_t hash) {
    s_pipelineStatePool.Release(hash);
  }

  virtual RenderPass* GetOrCreateRenderPass(
      const std::vector<Attachment>& colorAttachments,
      const math::Vector2Di&          offset,
      const math::Vector2Di&          extent) const override {
    return s_renderPassPool.GetOrCreate(
        RenderPassVk(colorAttachments, offset, extent));
  }

  virtual RenderPass* GetOrCreateRenderPass(
      const std::vector<Attachment>& colorAttachments,
      const Attachment&              depthAttachment,
      const math::Vector2Di&          offset,
      const math::Vector2Di&          extent) const override {
    return s_renderPassPool.GetOrCreate(
        RenderPassVk(colorAttachments, depthAttachment, offset, extent));
  }

  virtual RenderPass* GetOrCreateRenderPass(
      const std::vector<Attachment>& colorAttachments,
      const Attachment&              depthAttachment,
      const Attachment&              colorResolveAttachment,
      const math::Vector2Di&          offset,
      const math::Vector2Di&          extent) const override {
    return s_renderPassPool.GetOrCreate(RenderPassVk(colorAttachments,
                                                   depthAttachment,
                                                   colorResolveAttachment,
                                                   offset,
                                                   extent));
  }

  virtual RenderPass* GetOrCreateRenderPass(
      const RenderPassInfo& renderPassInfo,
      const math::Vector2Di& offset,
      const math::Vector2Di& extent) const override {
    return s_renderPassPool.GetOrCreate(
        RenderPassVk(renderPassInfo, offset, extent));
  }

  virtual MemoryPoolVk* GetMemoryPool() const override { return m_memoryPool_; }

  DescriptorPoolVk* GetDescriptorPoolForSingleFrame() const {
    return m_descriptorPoolsSingleFrame_[m_currentFrameIndex_];
  }

  DescriptorPoolVk* GetDescriptorPoolMultiFrame() const {
    return m_descriptorPoolMultiFrame_;
  }

  RingBufferVk* GetOneFrameUniformRingBuffer() const {
    return m_oneFrameUniformRingBuffers_[m_currentFrameIndex_];
  }

  virtual CommandBufferManagerVk* GetCommandBufferManager() const override {
    return m_commandBufferManager_;
  }

  virtual SemaphoreManagerVk* GetSemaphoreManager() override {
    return &m_semaphoreManager_;
  }

  virtual FenceManagerVk* GetFenceManager() override { return m_fenceManager_; }

  virtual std::shared_ptr<Swapchain> GetSwapchain() const override {
    return m_swapchain_;
  }

  virtual uint32_t GetCurrentFrameIndex() const override {
    return m_currentFrameIndex_;
  }

  virtual EMSAASamples GetSelectedMSAASamples() const override {
    return m_selectedMSAASamples_;
  }

  virtual uint32_t GetCurrentFrameNumber() const override {
    return m_currentFrameNumber_;
  }

  virtual bool OnHandleResized(uint32_t witdh,
                               uint32_t height,
                               bool     isMinimized) override {
    assert(witdh > 0);
    assert(height > 0);

    Finish();

    m_swapchain_->Create(m_window_);

    return true;
  }

  virtual void IncrementFrameNumber() { ++m_currentFrameNumber_; }

  void DrawArrays(
      const std::shared_ptr<RenderFrameContext>& renderFrameContext,
      /*EPrimitiveType                               type, - deprecated (used in
         previous rendering api)*/
      int32_t                                    vertStartIndex,
      int32_t                                    vertCount) const override;

  void DrawArraysInstanced(
      const std::shared_ptr<RenderFrameContext>& renderFrameContext,
      /*EPrimitiveType                               type, - deprecated (used in
         previous rendering api)*/
      int32_t                                    vertStartIndex,
      int32_t                                    vertCount,
      int32_t                                    instanceCount) const override;

  void DrawElements(
      const std::shared_ptr<RenderFrameContext>& renderFrameContext,
      /*EPrimitiveType                               type, - deprecated (used in
         previous rendering api)*/
      int32_t                                    elementSize,
      int32_t                                    startIndex,
      int32_t                                    indexCount) const override;

  void DrawElementsInstanced(
      const std::shared_ptr<RenderFrameContext>& renderFrameContext,
      /*EPrimitiveType                               type, - deprecated (used in
         previous rendering api)*/
      int32_t                                    elementSize,
      int32_t                                    startIndex,
      int32_t                                    indexCount,
      int32_t                                    instanceCount) const override;

  void DrawElementsBaseVertex(
      const std::shared_ptr<RenderFrameContext>& renderFrameContext,
      /*EPrimitiveType                               type, - deprecated (used in
         previous rendering api)*/
      int32_t                                    elementSize,
      int32_t                                    startIndex,
      int32_t                                    indexCount,
      int32_t baseVertexIndex) const override;

  void DrawElementsInstancedBaseVertex(
      const std::shared_ptr<RenderFrameContext>& renderFrameContext,
      /*EPrimitiveType                               type, - deprecated (used in
         previous rendering api)*/
      int32_t                                    elementSize,
      int32_t                                    startIndex,
      int32_t                                    indexCount,
      int32_t                                    baseVertexIndex,
      int32_t                                    instanceCount) const override;

  void DrawIndirect(
      const std::shared_ptr<RenderFrameContext>& renderFrameContext,
      /*EPrimitiveType                               type, - deprecated (used in
         previous rendering api)*/
      Buffer*                                     buffer,
      int32_t                                     startIndex,
      int32_t                                     drawCount) const override;

  void DrawElementsIndirect(
      const std::shared_ptr<RenderFrameContext>& renderFrameContext,
      /*EPrimitiveType                               type, - deprecated (used in
         previous rendering api)*/
      Buffer*                                     buffer,
      int32_t                                     startIndex,
      int32_t                                     drawCount) const override;

  void DispatchCompute(
      const std::shared_ptr<RenderFrameContext>& renderFrameContext,
      uint32_t                                    numGroupsX,
      uint32_t                                    numGroupsY,
      uint32_t                                    numGroupsZ) const override;

  void Flush() const override;

  void Finish() const override;

  void RecreateSwapChain() override;

  virtual std::shared_ptr<RenderFrameContext> BeginRenderFrame() override;

  virtual void EndRenderFrame(const std::shared_ptr<RenderFrameContext>&
                                  renderFrameContextPtr) override;

  void QueueSubmit(
      const std::shared_ptr<RenderFrameContext>& renderFrameContextPtr,
      Semaphore*                                  InSignalSemaphore) override;

  virtual CommandBufferVk* BeginSingleTimeCommands() const override;

  void EndSingleTimeCommands(CommandBuffer* commandBuffer) const override;

  virtual void BindGraphicsShaderBindingInstances(
      const CommandBuffer*                 commandBuffer,
      const PipelineStateInfo*            piplineState,
      const ShaderBindingInstanceCombiner& shaderBindingInstanceCombiner,
      std::uint32_t                        firstSet) const override;

  // TODO: add methods for VkBufferMemoryBarrier transition layout

  bool TransitionLayout(VkCommandBuffer commandBuffer,
                        VkImage         image,
                        VkFormat        format,
                        uint32_t        mipLevels,
                        uint32_t        layoutCount,
                        VkImageLayout   oldLayout,
                        VkImageLayout   newLayout) const;

  virtual bool TransitionLayout(CommandBuffer*  commandBuffer,
                                Texture*        texture,
                                EResourceLayout newLayout) const override;

  virtual void BindComputeShaderBindingInstances(
      const CommandBuffer*                 commandBuffer,
      const PipelineStateInfo*            piplineState,
      const ShaderBindingInstanceCombiner& shaderBindingInstanceCombiner,
      std::uint32_t                        firstSet) const override;

  // TODO: currently not used
  virtual void NextSubpass(const CommandBuffer* commandBuffer) const override;

  // TODO: uncomment
  // private:
  const std::vector<const char*> m_deviceExtensions_
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
    uint32_t m_queueIndex_ = 0;
    VkQueue  m_queue_      = nullptr;
  };

  QueueVk m_graphicsQueue_;
  QueueVk m_computeQueue_;
  QueueVk m_presentQueue_;

  VkDebugUtilsMessengerEXT m_debugMessenger_ = nullptr;

  std::shared_ptr<SwapchainVk> m_swapchain_ = std::make_shared<SwapchainVk>();
  // const bool  isVSyncEnabled{true};

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

  static std::unordered_map<size_t, ShaderBindingLayoutVk*> s_shaderBindingPool;
  static TResourcePool<SamplerStateInfoVk, MutexRWLock>     s_samplerStatePool;
  static TResourcePool<RasterizationStateInfoVk, MutexRWLock>
      s_rasterizationStatePool;
  static TResourcePool<StencilOpStateInfoVk, MutexRWLock> s_stencilOpStatePool;
  static TResourcePool<DepthStencilStateInfoVk, MutexRWLock>
                                                         s_depthStencilStatePool;
  static TResourcePool<BlendingStateInfoVk, MutexRWLock> s_blendingStatePool;
  static TResourcePool<PipelineStateInfoVk, MutexRWLock> s_pipelineStatePool;
  static TResourcePool<RenderPassVk, MutexRWLock>        s_renderPassPool;

  bool m_isFramebufferResized_ = false;

  EMSAASamples m_selectedMSAASamples_ = EMSAASamples::COUNT_1;
};

extern RhiVk* g_rhi_vk;


}  // namespace game_engine

#endif  // GAME_ENGINE_RHI_VK_H