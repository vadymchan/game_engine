#ifndef GAME_ENGINE_RHI_DX12_H
#define GAME_ENGINE_RHI_DX12_H

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

class jSwapchain_DX12;
struct jBuffer_DX12;
struct jTexture_DX12;
struct jBuffer_DX12;
// struct jRingBuffer_DX12;
struct jVertexBuffer_DX12;
struct jIndexBuffer_DX12;
struct jPipelineStateInfo_DX12;

using jDeallocatorMultiFrameCreatedResource
    = game_engine::jDeallocatorMultiFrameResource<ComPtr<ID3D12Resource>>;

namespace game_engine {

static const uint32_t        cbvCountPerFrame = 3;
static constexpr DXGI_FORMAT BackbufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

struct jPlacedResource {
  bool IsValid() const { return Size > 0 && PlacedSubResource.Get(); }

  ComPtr<ID3D12Resource> PlacedSubResource;
  size_t                 Offset           = 0;
  size_t                 Size             = 0;
  bool                   IsUploadResource = false;
};

// Resuse for PlacedResource for DX12
struct jPlacedResourcePool {
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

  static constexpr uint64_t MemorySize[(int32_t)EPoolSizeType::MAX] = {
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

  void Init();
  void Release();

  const jPlacedResource Alloc(size_t InRequestedSize, bool InIsUploadResource) {
    ScopedLock s(&Lock);

    auto& PendingList
        = GetPendingPlacedResources(InIsUploadResource, InRequestedSize);
    for (int32_t i = 0; i < (int32_t)PendingList.size(); ++i) {
      if (PendingList[i].Size >= InRequestedSize) {
        jPlacedResource resource = PendingList[i];
        PendingList.erase(PendingList.begin() + i);
        UsingPlacedResources.insert(
            std::make_pair(resource.PlacedSubResource.Get(), resource));
        return resource;
      }
    }

    return jPlacedResource();
  }

  void Free(const ComPtr<ID3D12Resource>& InData);

  void AddUsingPlacedResource(const jPlacedResource InPlacedResource) {
    assert(InPlacedResource.IsValid());
    {
      ScopedLock s(&Lock);
      UsingPlacedResources.insert(std::make_pair(
          InPlacedResource.PlacedSubResource.Get(), InPlacedResource));
    }
  }

  // This will be called from 'jDeallocatorMultiFrameUniformBufferBlock'
  // TODO: consider remove nested smart pointers (probably need changes in
  // jDeallocatorMultiFrameResource)
  void FreedFromPendingDelegateForCreatedResource(
      const std::shared_ptr<ComPtr<ID3D12Resource>>& InData) {
    Free(InData.get()->Get());
  }

  std::vector<jPlacedResource>& GetPendingPlacedResources(
      bool InIsUploadPlacedResource, size_t InSize) {
    const int32_t Index = (int32_t)GetPoolSizeType(InSize);
    assert(Index != (int32_t)EPoolSizeType::MAX);
    return InIsUploadPlacedResource ? PendingUploadPlacedResources[Index]
                                    : PendingPlacedResources[Index];
  }

  // PoolSize
  EPoolSizeType GetPoolSizeType(uint64_t InSize) const {
    for (int32_t i = 0; i < (int32_t)EPoolSizeType::MAX; ++i) {
      if (MemorySize[i] > InSize) {
        return (EPoolSizeType)i;
      }
    }
    return EPoolSizeType::MAX;
  }

  MutexLock                                  Lock;
  std::map<ID3D12Resource*, jPlacedResource> UsingPlacedResources;
  std::vector<jPlacedResource>
      PendingPlacedResources[(int32_t)EPoolSizeType::MAX];
  std::vector<jPlacedResource>
      PendingUploadPlacedResources[(int32_t)EPoolSizeType::MAX];
};

class jRHI_DX12 : public jRHI {
  public:
  static constexpr UINT MaxFrameCount = 3;

  jRHI_DX12();
  virtual ~jRHI_DX12();

  //////////////////////////////////////////////////////////////////////////
  // 1. Device
  ComPtr<IDXGIAdapter3> Adapter;
  ComPtr<ID3D12Device5> Device;
  uint32_t              Options = 0;
  ComPtr<IDXGIFactory5> Factory;

  //////////////////////////////////////////////////////////////////////////
  // 2. Command
  jCommandBufferManager_DX12* CommandBufferManager     = nullptr;
  jCommandBufferManager_DX12* CopyCommandBufferManager = nullptr;

  //////////////////////////////////////////////////////////////////////////
  // 3. Swapchain
  std::shared_ptr<jSwapchain_DX12> Swapchain
      = std::make_shared<jSwapchain_DX12>();
  uint32_t CurrentFrameIndex = 0;

  //////////////////////////////////////////////////////////////////////////
  // 4. Heap
  jOfflineDescriptorHeap_DX12 RTVDescriptorHeaps;
  jOfflineDescriptorHeap_DX12 DSVDescriptorHeaps;
  jOfflineDescriptorHeap_DX12 DescriptorHeaps;
  jOfflineDescriptorHeap_DX12 SamplerDescriptorHeaps;

  jOnlineDescriptorManager OnlineDescriptorHeapManager;

  // 7. Create sync object
  jFenceManager_DX12 FenceManager;
  void               WaitForGPU() const;

  HWND m_hWnd = 0;

  float m_focalDistance = 10.0f;
  float m_lensRadius    = 0.2f;

  //////////////////////////////////////////////////////////////////////////

  virtual Name GetRHIName() override { return NameStatic("DirectX12"); }

  virtual bool init(const std::shared_ptr<Window>& window) override;
  virtual void release() override;

  uint32_t     AdapterID = -1;
  std::wstring AdapterName;

  //HWND CreateMainWindow(const std::shared_ptr<Window>& window) const;

  //////////////////////////////////////////////////////////////////////////
  // PlacedResouce test
  ComPtr<ID3D12Heap> PlacedResourceDefaultHeap;
  uint64_t           PlacedResourceDefaultHeapOffset = 0;
  MutexLock          PlacedPlacedResourceDefaultHeapOffsetLock;

  ComPtr<ID3D12Heap> PlacedResourceUploadHeap;
  uint64_t           PlacedResourceDefaultUploadOffset = 0;
  MutexLock          PlacedResourceDefaultUploadOffsetLock;

  static constexpr uint64_t GDefaultPlacedResourceHeapSize = 256 * 1024 * 1024;
  static constexpr uint64_t GPlacedResourceSizeThreshold   = 512 * 512 * 4;
  static constexpr bool     GIsUsePlacedResource           = true;

  jPlacedResourcePool PlacedResourcePool;

  template <typename T>
  std::shared_ptr<jCreatedResource> CreateResource(
      T&&                   InDesc,
      D3D12_RESOURCE_STATES InResourceState,
      D3D12_CLEAR_VALUE*    InClearValue = nullptr) {
    assert(Device);

    if (GIsUsePlacedResource) {
      const D3D12_RESOURCE_ALLOCATION_INFO info
          = Device->GetResourceAllocationInfo(0, 1, InDesc);

      jPlacedResource ReusePlacedResource
          = PlacedResourcePool.Alloc(info.SizeInBytes, false);
      if (ReusePlacedResource.IsValid()) {
        return jCreatedResource::CreatedFromResourcePool(
            ReusePlacedResource.PlacedSubResource);
      } else {
        ScopedLock s(&PlacedPlacedResourceDefaultHeapOffsetLock);
        const bool IsAvailableCreatePlacedResource
            = GIsUsePlacedResource
           && (info.SizeInBytes <= GPlacedResourceSizeThreshold)
           && ((PlacedResourceDefaultHeapOffset + info.SizeInBytes)
               <= GDefaultPlacedResourceHeapSize);

        if (IsAvailableCreatePlacedResource) {
          assert(PlacedResourceDefaultHeap);

          ComPtr<ID3D12Resource> NewResource;
          HRESULT                hr
              = Device->CreatePlacedResource(PlacedResourceDefaultHeap.Get(),
                                             PlacedResourceDefaultHeapOffset,
                                             std::forward<T>(InDesc),
                                             InResourceState,
                                             InClearValue,
                                             IID_PPV_ARGS(&NewResource));
          assert(SUCCEEDED(hr));

          PlacedResourceDefaultHeapOffset += info.SizeInBytes;

          jPlacedResource NewPlacedResource;
          NewPlacedResource.IsUploadResource  = false;
          NewPlacedResource.PlacedSubResource = NewResource;
          NewPlacedResource.Size              = info.SizeInBytes;
          PlacedResourcePool.AddUsingPlacedResource(NewPlacedResource);

          return jCreatedResource::CreatedFromResourcePool(
              NewPlacedResource.PlacedSubResource);
        }
      }
    }

    const CD3DX12_HEAP_PROPERTIES& HeapProperties
        = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    ComPtr<ID3D12Resource> NewResource;
    HRESULT                hr = Device->CreateCommittedResource(&HeapProperties,
                                                 D3D12_HEAP_FLAG_NONE,
                                                 std::forward<T>(InDesc),
                                                 InResourceState,
                                                 InClearValue,
                                                 IID_PPV_ARGS(&NewResource));
    assert(SUCCEEDED(hr));
    return jCreatedResource::CreatedFromStandalone(NewResource);
  }

  template <typename T>
  std::shared_ptr<jCreatedResource> CreateUploadResource(
      T&&                   InDesc,
      D3D12_RESOURCE_STATES InResourceState,
      D3D12_CLEAR_VALUE*    InClearValue = nullptr) {
    assert(Device);

    if (GIsUsePlacedResource) {
      const D3D12_RESOURCE_ALLOCATION_INFO info
          = Device->GetResourceAllocationInfo(0, 1, InDesc);

      jPlacedResource ReusePlacedUploadResource
          = PlacedResourcePool.Alloc(info.SizeInBytes, true);
      if (ReusePlacedUploadResource.IsValid()) {
        return jCreatedResource::CreatedFromResourcePool(
            ReusePlacedUploadResource.PlacedSubResource);
      } else {
        ScopedLock s(&PlacedResourceDefaultUploadOffsetLock);
        const bool IsAvailablePlacedResource
            = GIsUsePlacedResource
           && (info.SizeInBytes <= GPlacedResourceSizeThreshold)
           && ((PlacedResourceDefaultUploadOffset + info.SizeInBytes)
               <= GDefaultPlacedResourceHeapSize);

        if (IsAvailablePlacedResource) {
          assert(PlacedResourceUploadHeap);

          ComPtr<ID3D12Resource> NewResource;
          HRESULT                hr
              = Device->CreatePlacedResource(PlacedResourceUploadHeap.Get(),
                                             PlacedResourceDefaultUploadOffset,
                                             std::forward<T>(InDesc),
                                             InResourceState,
                                             InClearValue,
                                             IID_PPV_ARGS(&NewResource));
          assert(SUCCEEDED(hr));

          PlacedResourceDefaultUploadOffset += info.SizeInBytes;

          jPlacedResource NewPlacedResource;
          NewPlacedResource.IsUploadResource  = true;
          NewPlacedResource.PlacedSubResource = NewResource;
          NewPlacedResource.Size              = info.SizeInBytes;
          PlacedResourcePool.AddUsingPlacedResource(NewPlacedResource);

          return jCreatedResource::CreatedFromResourcePool(NewResource);
        }
      }
    }

    const CD3DX12_HEAP_PROPERTIES& HeapProperties
        = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    ComPtr<ID3D12Resource> NewResource;
    HRESULT                hr = Device->CreateCommittedResource(&HeapProperties,
                                                 D3D12_HEAP_FLAG_NONE,
                                                 std::forward<T>(InDesc),
                                                 InResourceState,
                                                 InClearValue,
                                                 IID_PPV_ARGS(&NewResource));

    assert(SUCCEEDED(hr));
    return jCreatedResource::CreatedFromStandalone(NewResource);
  }

  //////////////////////////////////////////////////////////////////////////

  virtual bool OnHandleResized(uint32_t InWidth,
                               uint32_t InHeight,
                               bool     InIsMinimized) override;

  virtual jCommandBuffer_DX12* BeginSingleTimeCommands() const override;
  virtual void                 EndSingleTimeCommands(
                      jCommandBuffer* commandBuffer) const override;

  jCommandBuffer_DX12* BeginSingleTimeCopyCommands() const;
  void EndSingleTimeCopyCommands(jCommandBuffer_DX12* commandBuffer) const;

  virtual std::shared_ptr<jTexture> CreateTextureFromData(
      const ImageData* InImageData) const override;

  virtual jFenceManager* GetFenceManager() override { return &FenceManager; }

  std::vector<jRingBuffer_DX12*> OneFrameUniformRingBuffers;

  jRingBuffer_DX12* GetOneFrameUniformRingBuffer() const {
    return OneFrameUniformRingBuffers[CurrentFrameIndex];
  }

  virtual jShaderBindingLayout* CreateShaderBindings(
      const jShaderBindingArray& InShaderBindingArray) const override;

  virtual jSamplerStateInfo* CreateSamplerState(
      const jSamplerStateInfo& initializer) const override;
  virtual jRasterizationStateInfo* CreateRasterizationState(
      const jRasterizationStateInfo& initializer) const override;
  virtual jStencilOpStateInfo* CreateStencilOpStateInfo(
      const jStencilOpStateInfo& initializer) const override;
  virtual jDepthStencilStateInfo* CreateDepthStencilState(
      const jDepthStencilStateInfo& initializer) const override;
  virtual jBlendingStateInfo* CreateBlendingState(
      const jBlendingStateInfo& initializer) const override;

  uint32_t CurrentFrameNumber
      = 0;  // FrameNumber is just Incremented frame by frame.

  virtual void IncrementFrameNumber() { ++CurrentFrameNumber; }

  virtual uint32_t GetCurrentFrameNumber() const override {
    return CurrentFrameNumber;
  }

  virtual uint32_t GetCurrentFrameIndex() const { return CurrentFrameIndex; }

  static std::unordered_map<size_t, jShaderBindingLayout*> ShaderBindingPool;
  mutable MutexRWLock ShaderBindingPoolLock;

  static TResourcePool<jSamplerStateInfo_DX12, MutexRWLock> SamplerStatePool;
  static TResourcePool<jRasterizationStateInfo_DX12, MutexRWLock>
      RasterizationStatePool;
  static TResourcePool<jStencilOpStateInfo_DX12, MutexRWLock>
      StencilOpStatePool;
  static TResourcePool<jDepthStencilStateInfo_DX12, MutexRWLock>
      DepthStencilStatePool;
  static TResourcePool<jBlendingStateInfo_DX12, MutexRWLock> BlendingStatePool;
  static TResourcePool<jPipelineStateInfo_DX12, MutexRWLock> PipelineStatePool;
  static TResourcePool<jRenderPass_DX12, MutexRWLock>        RenderPassPool;

  virtual bool CreateShaderInternal(
      Shader* OutShader, const ShaderInfo& shaderInfo) const override;

  virtual jRenderPass* GetOrCreateRenderPass(
      const std::vector<jAttachment>& colorAttachments,
      const math::Vector2Di&          offset,
      const math::Vector2Di&          extent) const override;
  virtual jRenderPass* GetOrCreateRenderPass(
      const std::vector<jAttachment>& colorAttachments,
      const jAttachment&              depthAttachment,
      const math::Vector2Di&          offset,
      const math::Vector2Di&          extent) const override;
  virtual jRenderPass* GetOrCreateRenderPass(
      const std::vector<jAttachment>& colorAttachments,
      const jAttachment&              depthAttachment,
      const jAttachment&              colorResolveAttachment,
      const math::Vector2Di&          offset,
      const math::Vector2Di&          extent) const override;
  virtual jRenderPass* GetOrCreateRenderPass(
      const jRenderPassInfo& renderPassInfo,
      const math::Vector2Di& offset,
      const math::Vector2Di& extent) const override;

  virtual jPipelineStateInfo* CreatePipelineStateInfo(
      const jPipelineStateFixedInfo*   InPipelineStateFixed,
      const GraphicsPipelineShader     InShader,
      const jVertexBufferArray&        InVertexBufferArray,
      const jRenderPass*               InRenderPass,
      const jShaderBindingLayoutArray& InShaderBindingArray,
      const jPushConstant*             InPushConstant,
      int32_t                          InSubpassIndex) const override;
  virtual jPipelineStateInfo* CreateComputePipelineStateInfo(
      const Shader*                    shader,
      const jShaderBindingLayoutArray& InShaderBindingArray,
      const jPushConstant*             pushConstant) const override;

  virtual void RemovePipelineStateInfo(size_t InHash) override;

  virtual std::shared_ptr<jRenderFrameContext> BeginRenderFrame() override;
  virtual void EndRenderFrame(const std::shared_ptr<jRenderFrameContext>&
                                  renderFrameContextPtr) override;

  virtual jCommandBufferManager_DX12* GetCommandBufferManager() const override {
    return CommandBufferManager;
  }

  virtual jCommandBufferManager_DX12* GetCopyCommandBufferManager() const {
    return CopyCommandBufferManager;
  }

  virtual void BindGraphicsShaderBindingInstances(
      const jCommandBuffer*                InCommandBuffer,
      const jPipelineStateInfo*            InPiplineState,
      const ShaderBindingInstanceCombiner& InShaderBindingInstanceCombiner,
      uint32_t                             InFirstSet) const override;
  virtual void BindComputeShaderBindingInstances(
      const jCommandBuffer*                InCommandBuffer,
      const jPipelineStateInfo*            InPiplineState,
      const ShaderBindingInstanceCombiner& InShaderBindingInstanceCombiner,
      uint32_t                             InFirstSet) const override;
  virtual void BindRaytracingShaderBindingInstances(
      const jCommandBuffer*                InCommandBuffer,
      const jPipelineStateInfo*            InPiplineState,
      const ShaderBindingInstanceCombiner& InShaderBindingInstanceCombiner,
      uint32_t                             InFirstSet) const override;

  virtual std::shared_ptr<jShaderBindingInstance> CreateShaderBindingInstance(
      const jShaderBindingArray&       InShaderBindingArray,
      const jShaderBindingInstanceType InType) const override;

  virtual void DrawArrays(
      const std::shared_ptr<jRenderFrameContext>& InRenderFrameContext,
      // EPrimitiveType                              type,
      int32_t                                     vertStartIndex,
      int32_t                                     vertCount) const override;
  virtual void DrawArraysInstanced(
      const std::shared_ptr<jRenderFrameContext>& InRenderFrameContext,
      // EPrimitiveType                              type,
      int32_t                                     vertStartIndex,
      int32_t                                     vertCount,
      int32_t                                     instanceCount) const override;
  virtual void DrawElements(
      const std::shared_ptr<jRenderFrameContext>& InRenderFrameContext,
      // EPrimitiveType                              type,
      int32_t                                     elementSize,
      int32_t                                     startIndex,
      int32_t                                     indexCount) const override;
  virtual void DrawElementsInstanced(
      const std::shared_ptr<jRenderFrameContext>& InRenderFrameContext,
      // EPrimitiveType                              type,
      int32_t                                     elementSize,
      int32_t                                     startIndex,
      int32_t                                     indexCount,
      int32_t                                     instanceCount) const override;
  virtual void DrawElementsBaseVertex(
      const std::shared_ptr<jRenderFrameContext>& InRenderFrameContext,
      // EPrimitiveType                              type,
      int32_t                                     elementSize,
      int32_t                                     startIndex,
      int32_t                                     indexCount,
      int32_t baseVertexIndex) const override;
  virtual void DrawElementsInstancedBaseVertex(
      const std::shared_ptr<jRenderFrameContext>& InRenderFrameContext,
      // EPrimitiveType                              type,
      int32_t                                     elementSize,
      int32_t                                     startIndex,
      int32_t                                     indexCount,
      int32_t                                     baseVertexIndex,
      int32_t                                     instanceCount) const override;
  virtual void DrawIndirect(
      const std::shared_ptr<jRenderFrameContext>& InRenderFrameContext,
      // EPrimitiveType                              type,
      jBuffer*                                    buffer,
      int32_t                                     startIndex,
      int32_t                                     drawCount) const override;
  virtual void DrawElementsIndirect(
      const std::shared_ptr<jRenderFrameContext>& InRenderFrameContext,
      // EPrimitiveType                              type,
      jBuffer*                                    buffer,
      int32_t                                     startIndex,
      int32_t                                     drawCount) const override;
  virtual void DispatchCompute(
      const std::shared_ptr<jRenderFrameContext>& InRenderFrameContext,
      uint32_t                                    numGroupsX,
      uint32_t                                    numGroupsY,
      uint32_t                                    numGroupsZ) const override;

  virtual void* GetWindow() const override { return m_hWnd; }

  virtual std::shared_ptr<jRenderTarget> CreateRenderTarget(
      const jRenderTargetInfo& info) const override;

  // Resource Barrier
  bool         TransitionLayout_Internal(jCommandBuffer*       commandBuffer,
                                         ID3D12Resource*       resource,
                                         D3D12_RESOURCE_STATES srcLayout,
                                         D3D12_RESOURCE_STATES dstLayout) const;
  virtual bool TransitionLayout(jCommandBuffer* commandBuffer,
                                jTexture*       texture,
                                EResourceLayout newLayout) const override;
  virtual bool TransitionLayoutImmediate(
      jTexture* texture, EResourceLayout newLayout) const override;
  virtual bool TransitionLayout(jCommandBuffer* commandBuffer,
                                jBuffer*        buffer,
                                EResourceLayout newLayout) const override;
  virtual bool TransitionLayoutImmediate(
      jBuffer* buffer, EResourceLayout newLayout) const override;

  virtual void UAVBarrier(jCommandBuffer* commandBuffer,
                          jTexture*       texture) const override;
  virtual void UAVBarrierImmediate(jTexture* texture) const override;
  virtual void UAVBarrier(jCommandBuffer* commandBuffer,
                          jBuffer*        buffer) const override;
  virtual void UAVBarrierImmediate(jBuffer* buffer) const override;

  //////////////////////////////////////////////////////////////////////////

  virtual std::shared_ptr<jSwapchain> GetSwapchain() const override {
    return Swapchain;
  }

  virtual jSwapchainImage* GetSwapchainImage(int32_t InIndex) const override {
    return Swapchain->GetSwapchainImage(InIndex);
  }

  // TODO: implement
  // virtual void BeginDebugEvent(jCommandBuffer*        InCommandBuffer,
  //                             const char*            InName,
  //                             const math::Vector4Df& InColor
  //                             = math::ColorGreen) const override;

  // virtual void EndDebugEvent(jCommandBuffer* InCommandBuffer) const override;

  virtual void Flush() const override;
  virtual void Finish() const override;

  MutexLock MultiFrameShaderBindingInstanceLock;
  jDeallocatorMultiFrameShaderBindingInstance
      DeallocatorMultiFrameShaderBindingInstance;
  jDeallocatorMultiFrameCreatedResource DeallocatorMultiFramePlacedResource;
  jDeallocatorMultiFrameCreatedResource DeallocatorMultiFrameStandaloneResource;

  // Create Buffers
  virtual std::shared_ptr<jBuffer> CreateStructuredBuffer(
      uint64_t          InSize,
      uint64_t          InAlignment,
      uint64_t          InStride,
      EBufferCreateFlag InBufferCreateFlag,
      EResourceLayout   InInitialState,
      const void*       InData     = nullptr,
      uint64_t          InDataSize = 0
      /*, const wchar_t*    InResourceName = nullptr*/) const override;

  virtual std::shared_ptr<jBuffer> CreateRawBuffer(
      uint64_t          InSize,
      uint64_t          InAlignment,
      EBufferCreateFlag InBufferCreateFlag,
      EResourceLayout   InInitialState,
      const void*       InData     = nullptr,
      uint64_t          InDataSize = 0
      /*, const wchar_t*    InResourceName = nullptr*/) const override;

  virtual std::shared_ptr<jBuffer> CreateFormattedBuffer(
      uint64_t          InSize,
      uint64_t          InAlignment,
      ETextureFormat    InFormat,
      EBufferCreateFlag InBufferCreateFlag,
      EResourceLayout   InInitialState,
      const void*       InData     = nullptr,
      uint64_t          InDataSize = 0
      /*, const wchar_t*    InResourceName = nullptr*/) const override;

  virtual std::shared_ptr<IUniformBufferBlock> CreateUniformBufferBlock(
      Name         InName,
      LifeTimeType InLifeTimeType,
      size_t       InSize = 0) const override;
  virtual std::shared_ptr<jVertexBuffer> CreateVertexBuffer(
      const std::shared_ptr<VertexStreamData>& streamData) const override;
  virtual std::shared_ptr<jIndexBuffer> CreateIndexBuffer(
      const std::shared_ptr<IndexStreamData>& streamData) const override;
  //////////////////////////////////////////////////////////////////////////

  // Create Images
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
  //////////////////////////////////////////////////////////////////////////

  virtual bool IsSupportVSync() const override;

  private:
  // TODO: consider whether need in this place
  std::shared_ptr<Window> m_window_;
};

extern jRHI_DX12* g_rhi_dx12;

}  // namespace game_engine

#endif  // GAME_ENGINE_RHI_DX12_H