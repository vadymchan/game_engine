#ifndef GAME_ENGINE_RHI_DX12_H
#define GAME_ENGINE_RHI_DX12_H

#include "platform/windows/windows_platform_setup.h"

#ifdef GAME_ENGINE_RHI_DX12

#include "gfx/rhi/dx12/command_allocator_dx12.h"
#include "gfx/rhi/dx12/fence_dx12.h"
#include "gfx/rhi/dx12/pipeline_state_info_dx12.h"
#include "gfx/rhi/dx12/render_pass_dx12.h"
#include "gfx/rhi/dx12/ring_buffer_dx12.h"
#include "gfx/rhi/dx12/swapchain_dx12.h"
#include "gfx/rhi/dx12/texture_dx12.h"
#include "gfx/rhi/rhi.h"
#include "gfx/rhi/shader_binding_layout.h"
#include "platform/windows/windows_platform_setup.h"

#include <cassert>

class SwapchainDx12;
struct BufferDx12;
struct TextureDx12;
struct BufferDx12;
// struct RingBufferDx12;
struct VertexBufferDx12;
struct IndexBufferDx12;
struct PipelineStateInfoDx12;

using DeallocatorMultiFrameCreatedResource
    = game_engine::DeallocatorMultiFrameResource<ComPtr<ID3D12Resource>>;

namespace game_engine {

// TODO: consider remove (currently not used)
// static const uint32_t        s_kCbvCountPerFrame = 3;
// static constexpr DXGI_FORMAT s_kBackbufferFormat =
// DXGI_FORMAT_R8G8B8A8_UNORM;

// TODO: consider move to other file
struct PlacedResource {
  // ======= BEGIN: public misc methods =======================================

  bool isValid() const { return m_size_ > 0 && m_placedSubResource_.Get(); }

  // ======= END: public misc methods   =======================================

  // ======= BEGIN: public misc fields ========================================

  ComPtr<ID3D12Resource> m_placedSubResource_;
  // TODO: seems that this is not used
  size_t                 m_offset_           = 0;
  size_t                 m_size_             = 0;
  bool                   m_isUploadResource_ = false;

  // ======= END: public misc fields   ========================================
};

// TODO: consider move to other file
// Resuse for PlacedResource for DX12
struct PlacedResourcePool {
  // ======= BEGIN: public nested types =======================================

  enum class EPoolSizeType : uint8_t {
    E128,
    E256,
    E512,
    E1K,
    E2K,
    E4K,
    E8K,
    E16K,
    E1M,
    E10M,
    E100M,
    MAX
  };

  // ======= END: public nested types   =======================================

  // ======= BEGIN: public static fields ======================================

  static constexpr uint64_t s_kMemorySize[(int32_t)EPoolSizeType::MAX] = {
    128,                // E128
    256,                // E256
    512,                // E512
    1024,               // E1K
    2048,               // E2K
    4096,               // E4K
    8192,               // E8K
    16 * 1024,          // E16K
    1024 * 1024,        // E1M
    10 * 1024 * 1024,   // E10M
    100 * 1024 * 1024,  // E100M
  };

  // ======= END: public static fields   ======================================

  // ======= BEGIN: public getters ============================================

  std::vector<PlacedResource>& getPendingPlacedResources(
      bool isUploadPlacedResource, size_t size) {
    const int32_t kIndex = (int32_t)getPoolSizeType(size);
    assert(kIndex != (int32_t)EPoolSizeType::MAX);
    return isUploadPlacedResource ? m_pendingUploadPlacedResources_[kIndex]
                                  : m_pendingPlacedResources_[kIndex];
  }

  // PoolSize
  EPoolSizeType getPoolSizeType(uint64_t size) const {
    for (int32_t i = 0; i < (int32_t)EPoolSizeType::MAX; ++i) {
      if (s_kMemorySize[i] > size) {
        return (EPoolSizeType)i;
      }
    }
    return EPoolSizeType::MAX;
  }

  // ======= END: public getters   ============================================

  // ======= BEGIN: public misc methods =======================================

  void init();
  void release();

  const PlacedResource alloc(size_t requestedSize, bool isUploadResource) {
    ScopedLock s(&m_lock_);

    auto& pendingList
        = getPendingPlacedResources(isUploadResource, requestedSize);
    for (int32_t i = 0; i < (int32_t)pendingList.size(); ++i) {
      if (pendingList[i].m_size_ >= requestedSize) {
        PlacedResource resource = pendingList[i];
        pendingList.erase(pendingList.begin() + i);
        m_usingPlacedResources_.insert(
            std::make_pair(resource.m_placedSubResource_.Get(), resource));
        return resource;
      }
    }

    return PlacedResource();
  }

  void free(const ComPtr<ID3D12Resource>& data);

  void addUsingPlacedResource(const PlacedResource placedResource) {
    assert(placedResource.isValid());
    {
      ScopedLock s(&m_lock_);
      m_usingPlacedResources_.insert(std::make_pair(
          placedResource.m_placedSubResource_.Get(), placedResource));
    }
  }

  // This will be called from 'DeallocatorMultiFrameUniformBufferBlock'
  // TODO: consider remove nested smart pointers (probably need changes in
  // DeallocatorMultiFrameResource)
  void freedFromPendingDelegateForCreatedResource(
      const std::shared_ptr<ComPtr<ID3D12Resource>>& data) {
    free(data.get()->Get());
  }

  // ======= END: public misc methods   =======================================

  // ======= BEGIN: public misc fields ========================================

  MutexLock                                 m_lock_;
  std::map<ID3D12Resource*, PlacedResource> m_usingPlacedResources_;
  std::vector<PlacedResource>
      m_pendingPlacedResources_[(int32_t)EPoolSizeType::MAX];
  std::vector<PlacedResource>
      m_pendingUploadPlacedResources_[(int32_t)EPoolSizeType::MAX];

  // ======= END: public misc fields   ========================================
};

class RhiDx12 : public RHI {
  public:
  // ======= BEGIN: public static fields ======================================

  static constexpr UINT     s_kMaxFrameCount = 3;
  static constexpr uint64_t s_kDefaultPlacedResourceHeapSize
      = 256 * 1024 * 1024;
  static constexpr uint64_t s_kPlacedResourceSizeThreshold = 512 * 512 * 4;
  static constexpr bool     s_kIsUsePlacedResource         = true;

  static std::unordered_map<size_t, ShaderBindingLayout*> s_shaderBindingPool;
  mutable MutexRWLock m_shaderBindingPoolLock_;

  static TResourcePool<SamplerStateInfoDx12, MutexRWLock> s_samplerStatePool;
  static TResourcePool<RasterizationStateInfoDx12, MutexRWLock>
      s_rasterizationStatePool;
  static TResourcePool<StencilOpStateInfoDx12, MutexRWLock>
      s_stencilOpStatePool;
  static TResourcePool<DepthStencilStateInfoDx12, MutexRWLock>
      s_depthStencilStatePool;
  static TResourcePool<BlendingStateInfoDx12, MutexRWLock> s_blendingStatePool;
  static TResourcePool<PipelineStateInfoDx12, MutexRWLock> s_pipelineStatePool;
  static TResourcePool<RenderPassDx12, MutexRWLock>        s_renderPassPool;

  // ======= END: public static fields   ======================================

  // ======= BEGIN: public constructors =======================================

  RhiDx12();

  // ======= END: public constructors   =======================================

  // ======= BEGIN: public destructor =========================================

  virtual ~RhiDx12();

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overridden methods =================================

  virtual bool init(const std::shared_ptr<Window>& window) override;
  virtual void release() override;

  virtual bool onHandleResized(uint32_t witdh,
                               uint32_t height,
                               bool     isMinimized) override;

  virtual CommandBufferDx12* beginSingleTimeCommands() const override;
  virtual void               endSingleTimeCommands(
                    CommandBuffer* commandBuffer) const override;

  virtual std::shared_ptr<Texture> createTextureFromData(
      const ImageData* imageData) const override;

  virtual ShaderBindingLayout* createShaderBindings(
      const ShaderBindingArray& shaderBindingArray) const override;

  virtual SamplerStateInfo* createSamplerState(
      const SamplerStateInfo& initializer) const override;
  virtual RasterizationStateInfo* createRasterizationState(
      const RasterizationStateInfo& initializer) const override;
  virtual StencilOpStateInfo* createStencilOpStateInfo(
      const StencilOpStateInfo& initializer) const override;
  virtual DepthStencilStateInfo* createDepthStencilState(
      const DepthStencilStateInfo& initializer) const override;
  virtual BlendingStateInfo* createBlendingState(
      const BlendingStateInfo& initializer) const override;

  virtual void incrementFrameNumber() { ++m_currentFrameNumber_; }

  virtual bool createShaderInternal(
      Shader* shader, const ShaderInfo& shaderInfo) const override;

  virtual PipelineStateInfo* createPipelineStateInfo(
      const PipelineStateFixedInfo*   pipelineStateFixed,
      const GraphicsPipelineShader    shader,
      const VertexBufferArray&        vertexBufferArray,
      const RenderPass*               renderPass,
      const ShaderBindingLayoutArray& shaderBindingArray,
      const PushConstant*             pushConstant,
      int32_t                         subpassIndex) const override;
  virtual PipelineStateInfo* createComputePipelineStateInfo(
      const Shader*                   shader,
      const ShaderBindingLayoutArray& shaderBindingArray,
      const PushConstant*             pushConstant) const override;

  virtual void removePipelineStateInfo(size_t hash) override;

  virtual std::shared_ptr<RenderFrameContext> beginRenderFrame() override;
  virtual void endRenderFrame(const std::shared_ptr<RenderFrameContext>&
                                  renderFrameContextPtr) override;

  virtual void bindGraphicsShaderBindingInstances(
      const CommandBuffer*                 commandBuffer,
      const PipelineStateInfo*             piplineState,
      const ShaderBindingInstanceCombiner& shaderBindingInstanceCombiner,
      uint32_t                             firstSet) const override;
  virtual void bindComputeShaderBindingInstances(
      const CommandBuffer*                 commandBuffer,
      const PipelineStateInfo*             piplineState,
      const ShaderBindingInstanceCombiner& shaderBindingInstanceCombiner,
      uint32_t                             firstSet) const override;
  virtual void bindRaytracingShaderBindingInstances(
      const CommandBuffer*                 commandBuffer,
      const PipelineStateInfo*             piplineState,
      const ShaderBindingInstanceCombiner& shaderBindingInstanceCombiner,
      uint32_t                             firstSet) const override;

  virtual std::shared_ptr<ShaderBindingInstance> createShaderBindingInstance(
      const ShaderBindingArray&       shaderBindingArray,
      const ShaderBindingInstanceType type) const override;

  virtual void drawArrays(
      const std::shared_ptr<RenderFrameContext>& renderFrameContext,
      // EPrimitiveType                          type,
      int32_t                                    vertStartIndex,
      int32_t                                    vertCount) const override;
  virtual void drawArraysInstanced(
      const std::shared_ptr<RenderFrameContext>& renderFrameContext,
      // EPrimitiveType                          type,
      int32_t                                    vertStartIndex,
      int32_t                                    vertCount,
      int32_t                                    instanceCount) const override;
  virtual void drawElements(
      const std::shared_ptr<RenderFrameContext>& renderFrameContext,
      // EPrimitiveType                          type,
      int32_t                                    elementSize,
      int32_t                                    startIndex,
      int32_t                                    indexCount) const override;
  virtual void drawElementsInstanced(
      const std::shared_ptr<RenderFrameContext>& renderFrameContext,
      // EPrimitiveType                          type,
      int32_t                                    elementSize,
      int32_t                                    startIndex,
      int32_t                                    indexCount,
      int32_t                                    instanceCount) const override;
  virtual void drawElementsBaseVertex(
      const std::shared_ptr<RenderFrameContext>& renderFrameContext,
      // EPrimitiveType                          type,
      int32_t                                    elementSize,
      int32_t                                    startIndex,
      int32_t                                    indexCount,
      int32_t baseVertexIndex) const override;
  virtual void drawElementsInstancedBaseVertex(
      const std::shared_ptr<RenderFrameContext>& renderFrameContext,
      // EPrimitiveType                          type,
      int32_t                                    elementSize,
      int32_t                                    startIndex,
      int32_t                                    indexCount,
      int32_t                                    baseVertexIndex,
      int32_t                                    instanceCount) const override;
  virtual void drawIndirect(
      const std::shared_ptr<RenderFrameContext>& renderFrameContext,
      // EPrimitiveType                          type,
      IBuffer*                                   buffer,
      int32_t                                    startIndex,
      int32_t                                    drawCount) const override;
  virtual void drawElementsIndirect(
      const std::shared_ptr<RenderFrameContext>& renderFrameContext,
      // EPrimitiveType                          type,
      IBuffer*                                   buffer,
      int32_t                                    startIndex,
      int32_t                                    drawCount) const override;
  virtual void dispatchCompute(
      const std::shared_ptr<RenderFrameContext>& renderFrameContext,
      uint32_t                                   numGroupsX,
      uint32_t                                   numGroupsY,
      uint32_t                                   numGroupsZ) const override;

  virtual std::shared_ptr<RenderTarget> createRenderTarget(
      const RenderTargetInfo& info) const override;

  // Resource Barrier
  virtual bool transitionLayout(CommandBuffer*  commandBuffer,
                                Texture*        texture,
                                EResourceLayout newLayout) const override;
  virtual bool transitionLayoutImmediate(
      Texture* texture, EResourceLayout newLayout) const override;
  virtual bool transitionLayout(CommandBuffer*  commandBuffer,
                                IBuffer*        buffer,
                                EResourceLayout newLayout) const override;
  virtual bool transitionLayoutImmediate(
      IBuffer* buffer, EResourceLayout newLayout) const override;

  virtual void uavBarrier(CommandBuffer* commandBuffer,
                          Texture*       texture) const override;
  virtual void uavBarrierImmediate(Texture* texture) const override;
  virtual void uavBarrier(CommandBuffer* commandBuffer,
                          IBuffer*       buffer) const override;
  virtual void uavBarrierImmediate(IBuffer* buffer) const override;

  // TODO: implement
  // virtual void beginDebugEvent(CommandBuffer*        commandBuffer,
  //                             const char*            name,
  //                             const math::Vector4Df& color
  //                             = math::g_kColorGreen) const override;

  // virtual void endDebugEvent(CommandBuffer* commandBuffer) const override;

  virtual void flush() const override;
  virtual void finish() const override;

  // Create Buffers
  virtual std::shared_ptr<IBuffer> createStructuredBuffer(
      uint64_t          size,
      uint64_t          alignment,
      uint64_t          stride,
      EBufferCreateFlag bufferCreateFlag,
      EResourceLayout   initialState,
      const void*       data     = nullptr,
      uint64_t          dataSize = 0
      /*, const wchar_t*    resourceName = nullptr*/) const override;

  virtual std::shared_ptr<IBuffer> createRawBuffer(
      uint64_t          size,
      uint64_t          alignment,
      EBufferCreateFlag bufferCreateFlag,
      EResourceLayout   initialState,
      const void*       data     = nullptr,
      uint64_t          dataSize = 0
      /*, const wchar_t*    resourceName = nullptr*/) const override;

  virtual std::shared_ptr<IBuffer> createFormattedBuffer(
      uint64_t          size,
      uint64_t          alignment,
      ETextureFormat    format,
      EBufferCreateFlag bufferCreateFlag,
      EResourceLayout   initialState,
      const void*       data     = nullptr,
      uint64_t          dataSize = 0
      /*, const wchar_t*    resourceName = nullptr*/) const override;

  virtual std::shared_ptr<IUniformBufferBlock> createUniformBufferBlock(
      Name name, LifeTimeType lifeTimeType, size_t size = 0) const override;
  virtual std::shared_ptr<VertexBuffer> createVertexBuffer(
      const std::shared_ptr<VertexStreamData>& streamData) const override;
  virtual std::shared_ptr<IndexBuffer> createIndexBuffer(
      const std::shared_ptr<IndexStreamData>& streamData) const override;

  // Create Images
  virtual std::shared_ptr<Texture> create2DTexture(
      uint32_t             witdh,
      uint32_t             height,
      uint32_t             arrayLayers,
      uint32_t             mipLevels,
      ETextureFormat       format,
      ETextureCreateFlag   textureCreateFlag,
      EResourceLayout      imageLayout   = EResourceLayout::UNDEFINED,
      const ImageBulkData& imageBulkData = {},
      const RtClearValue&  clearValue    = RtClearValue::s_kInvalid,
      const wchar_t*       resourceName  = nullptr) const override;

  virtual std::shared_ptr<Texture> createCubeTexture(
      uint32_t             witdh,
      uint32_t             height,
      uint32_t             mipLevels,
      ETextureFormat       format,
      ETextureCreateFlag   textureCreateFlag,
      EResourceLayout      imageLayout   = EResourceLayout::UNDEFINED,
      const ImageBulkData& imageBulkData = {},
      const RtClearValue&  clearValue    = RtClearValue::s_kInvalid,
      const wchar_t*       resourceName  = nullptr) const override;

  virtual bool isSupportVSync() const override;

  virtual RenderPass* getOrCreateRenderPass(
      const std::vector<Attachment>& colorAttachments,
      const math::Vector2Di&         offset,
      const math::Vector2Di&         extent) const override;
  virtual RenderPass* getOrCreateRenderPass(
      const std::vector<Attachment>& colorAttachments,
      const Attachment&              depthAttachment,
      const math::Vector2Di&         offset,
      const math::Vector2Di&         extent) const override;
  virtual RenderPass* getOrCreateRenderPass(
      const std::vector<Attachment>& colorAttachments,
      const Attachment&              depthAttachment,
      const Attachment&              colorResolveAttachment,
      const math::Vector2Di&         offset,
      const math::Vector2Di&         extent) const override;
  virtual RenderPass* getOrCreateRenderPass(
      const RenderPassInfo&  renderPassInfo,
      const math::Vector2Di& offset,
      const math::Vector2Di& extent) const override;

  virtual Name getRHIName() override { return NameStatic("DirectX12"); }

  virtual IFenceManager* getFenceManager() override { return &m_fenceManager_; }

  virtual uint32_t getCurrentFrameNumber() const override {
    return m_currentFrameNumber_;
  }

  virtual uint32_t getCurrentFrameIndex() const { return m_currentFrameIndex_; }

  virtual CommandBufferManagerDx12* getCommandBufferManager() const override {
    return m_commandBufferManager_;
  }

  virtual CommandBufferManagerDx12* getCopyCommandBufferManager() const {
    return m_copyCommandBufferManager_;
  }

  virtual void* getWindow() const override { return m_hWnd; }

  virtual std::shared_ptr<ISwapchain> getSwapchain() const override {
    return m_swapchain_;
  }

  virtual ISwapchainImage* getSwapchainImage(int32_t index) const override {
    return m_swapchain_->getSwapchainImage(index);
  }

  // ======= END: public overridden methods   =================================

  // ======= BEGIN: public getters ============================================

  RingBufferDx12* getOneFrameUniformRingBuffer() const {
    return m_oneFrameUniformRingBuffers_[m_currentFrameIndex_];
  }

  // ======= END: public getters   ============================================

  // ======= BEGIN: public misc methods =======================================

  template <typename T>
  std::shared_ptr<CreatedResource> createResource(
      T&&                   desc,
      D3D12_RESOURCE_STATES resourceState,
      D3D12_CLEAR_VALUE*    clearValue = nullptr) {
    assert(m_device_);

    if (s_kIsUsePlacedResource) {
      const D3D12_RESOURCE_ALLOCATION_INFO info
          = m_device_->GetResourceAllocationInfo(0, 1, desc);

      PlacedResource ReusePlacedResource
          = m_placedResourcePool_.alloc(info.SizeInBytes, false);
      if (ReusePlacedResource.isValid()) {
        return CreatedResource::s_createdFromResourcePool(
            ReusePlacedResource.m_placedSubResource_);
      } else {
        ScopedLock s(&m_placedPlacedResourceDefaultHeapOffsetLock_);
        const bool IsAvailableCreatePlacedResource
            = s_kIsUsePlacedResource
           && (info.SizeInBytes <= s_kPlacedResourceSizeThreshold)
           && ((m_placedResourceDefaultHeapOffset_ + info.SizeInBytes)
               <= s_kDefaultPlacedResourceHeapSize);

        if (IsAvailableCreatePlacedResource) {
          assert(m_placedResourceDefaultHeap_);

          ComPtr<ID3D12Resource> NewResource;
          HRESULT                hr = m_device_->CreatePlacedResource(
              m_placedResourceDefaultHeap_.Get(),
              m_placedResourceDefaultHeapOffset_,
              std::forward<T>(desc),
              resourceState,
              clearValue,
              IID_PPV_ARGS(&NewResource));
          assert(SUCCEEDED(hr));

          m_placedResourceDefaultHeapOffset_ += info.SizeInBytes;

          PlacedResource NewPlacedResource;
          NewPlacedResource.m_isUploadResource_  = false;
          NewPlacedResource.m_placedSubResource_ = NewResource;
          NewPlacedResource.m_size_              = info.SizeInBytes;
          m_placedResourcePool_.addUsingPlacedResource(NewPlacedResource);

          return CreatedResource::s_createdFromResourcePool(
              NewPlacedResource.m_placedSubResource_);
        }
      }
    }

    const CD3DX12_HEAP_PROPERTIES& HeapProperties
        = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    ComPtr<ID3D12Resource> NewResource;
    HRESULT hr = m_device_->CreateCommittedResource(&HeapProperties,
                                                    D3D12_HEAP_FLAG_NONE,
                                                    std::forward<T>(desc),
                                                    resourceState,
                                                    clearValue,
                                                    IID_PPV_ARGS(&NewResource));
    assert(SUCCEEDED(hr));
    return CreatedResource::s_createdFromStandalone(NewResource);
  }

  template <typename T>
  std::shared_ptr<CreatedResource> createUploadResource(
      T&&                   desc,
      D3D12_RESOURCE_STATES resourceState,
      D3D12_CLEAR_VALUE*    clearValue = nullptr) {
    assert(m_device_);

    if (s_kIsUsePlacedResource) {
      const D3D12_RESOURCE_ALLOCATION_INFO info
          = m_device_->GetResourceAllocationInfo(0, 1, desc);

      PlacedResource ReusePlacedUploadResource
          = m_placedResourcePool_.alloc(info.SizeInBytes, true);
      if (ReusePlacedUploadResource.isValid()) {
        return CreatedResource::s_createdFromResourcePool(
            ReusePlacedUploadResource.m_placedSubResource_);
      } else {
        ScopedLock s(&m_placedResourceDefaultUploadOffsetLock_);
        const bool IsAvailablePlacedResource
            = s_kIsUsePlacedResource
           && (info.SizeInBytes <= s_kPlacedResourceSizeThreshold)
           && ((m_placedResourceDefaultUploadOffset_ + info.SizeInBytes)
               <= s_kDefaultPlacedResourceHeapSize);

        if (IsAvailablePlacedResource) {
          assert(m_placedResourceUploadHeap_);

          ComPtr<ID3D12Resource> NewResource;
          HRESULT                hr = m_device_->CreatePlacedResource(
              m_placedResourceUploadHeap_.Get(),
              m_placedResourceDefaultUploadOffset_,
              std::forward<T>(desc),
              resourceState,
              clearValue,
              IID_PPV_ARGS(&NewResource));
          assert(SUCCEEDED(hr));

          m_placedResourceDefaultUploadOffset_ += info.SizeInBytes;

          PlacedResource NewPlacedResource;
          NewPlacedResource.m_isUploadResource_  = true;
          NewPlacedResource.m_placedSubResource_ = NewResource;
          NewPlacedResource.m_size_              = info.SizeInBytes;
          m_placedResourcePool_.addUsingPlacedResource(NewPlacedResource);

          return CreatedResource::s_createdFromResourcePool(NewResource);
        }
      }
    }

    const CD3DX12_HEAP_PROPERTIES& HeapProperties
        = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    ComPtr<ID3D12Resource> NewResource;
    HRESULT hr = m_device_->CreateCommittedResource(&HeapProperties,
                                                    D3D12_HEAP_FLAG_NONE,
                                                    std::forward<T>(desc),
                                                    resourceState,
                                                    clearValue,
                                                    IID_PPV_ARGS(&NewResource));

    assert(SUCCEEDED(hr));
    return CreatedResource::s_createdFromStandalone(NewResource);
  }

  CommandBufferDx12* beginSingleTimeCopyCommands() const;
  void endSingleTimeCopyCommands(CommandBufferDx12* commandBuffer) const;

  // Resource Barrier
  bool transitionLayoutInternal(CommandBuffer*        commandBuffer,
                                ID3D12Resource*       resource,
                                D3D12_RESOURCE_STATES srcLayout,
                                D3D12_RESOURCE_STATES dstLayout) const;

  void waitForGPU() const;

  // ======= END: public misc methods   =======================================

  // ======= BEGIN: public misc fields ========================================

  // 1. Device
  ComPtr<IDXGIAdapter3> m_adapter_;
  ComPtr<ID3D12Device5> m_device_;
  // TODO: seems that this is not used
  uint32_t              m_options_ = 0;
  ComPtr<IDXGIFactory5> m_factory_;

  //////////////////////////////////////////////////////////////////////////
  // 2. Command
  CommandBufferManagerDx12* m_commandBufferManager_     = nullptr;
  CommandBufferManagerDx12* m_copyCommandBufferManager_ = nullptr;

  //////////////////////////////////////////////////////////////////////////
  // 3. Swapchain
  std::shared_ptr<SwapchainDx12> m_swapchain_
      = std::make_shared<SwapchainDx12>();
  uint32_t m_currentFrameIndex_ = 0;

  //////////////////////////////////////////////////////////////////////////
  // 4. Heap
  // TODO: consider whether the variables in correct naming conventions
  OfflineDescriptorHeapDx12 m_rtvDescriptorHeaps_;
  OfflineDescriptorHeapDx12 m_dsvDescriptorHeaps_;
  OfflineDescriptorHeapDx12 m_descriptorHeaps_;
  OfflineDescriptorHeapDx12 m_samplerDescriptorHeaps_;

  OnlineDescriptorManager m_onlineDescriptorHeapManager_;

  // 7. Create sync object
  FenceManagerDx12 m_fenceManager_;

  HWND m_hWnd = 0;

  // TODO: seems not used
  // float m_focalDistance_ = 10.0f;
  // float m_lensRadius_    = 0.2f;

  //////////////////////////////////////////////////////////////////////////

  uint32_t     m_adapterId_ = -1;
  std::wstring m_adapterName_;

  // HWND CreateMainWindow(const std::shared_ptr<Window>& window) const;

  //////////////////////////////////////////////////////////////////////////
  // PlacedResouce test
  ComPtr<ID3D12Heap> m_placedResourceDefaultHeap_;
  uint64_t           m_placedResourceDefaultHeapOffset_ = 0;
  MutexLock          m_placedPlacedResourceDefaultHeapOffsetLock_;

  ComPtr<ID3D12Heap> m_placedResourceUploadHeap_;
  uint64_t           m_placedResourceDefaultUploadOffset_ = 0;
  MutexLock          m_placedResourceDefaultUploadOffsetLock_;

  PlacedResourcePool m_placedResourcePool_;

  std::vector<RingBufferDx12*> m_oneFrameUniformRingBuffers_;

  uint32_t m_currentFrameNumber_
      = 0;  // FrameNumber is just Incremented frame by frame.

  MutexLock m_multiFrameShaderBindingInstanceLock_;
  DeallocatorMultiFrameShaderBindingInstance
      m_deallocatorMultiFrameShaderBindingInstance_;
  DeallocatorMultiFrameCreatedResource m_deallocatorMultiFramePlacedResource_;
  DeallocatorMultiFrameCreatedResource
      m_deallocatorMultiFrameStandaloneResource_;

  // ======= END: public misc fields   ========================================

  private:
  // ======= BEGIN: private misc fields =======================================

  // TODO: consider whether need in this place
  std::shared_ptr<Window> m_window_;

  // ======= END: private misc fields   =======================================
};

extern RhiDx12* g_rhiDx12;

}  // namespace game_engine

#endif  // GAME_ENGINE_RHI_DX12

#endif  // GAME_ENGINE_RHI_DX12_H