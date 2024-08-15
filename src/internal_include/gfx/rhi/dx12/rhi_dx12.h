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

static const uint32_t        cbvCountPerFrame = 3;
static constexpr DXGI_FORMAT BackbufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

struct PlacedResource {
  bool IsValid() const { return Size > 0 && PlacedSubResource.Get(); }

  ComPtr<ID3D12Resource> PlacedSubResource;
  size_t                 Offset           = 0;
  size_t                 Size             = 0;
  bool                   IsUploadResource = false;
};

// Resuse for PlacedResource for DX12
struct PlacedResourcePool {
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

  const PlacedResource Alloc(size_t InRequestedSize, bool InIsUploadResource) {
    ScopedLock s(&Lock);

    auto& PendingList
        = GetPendingPlacedResources(InIsUploadResource, InRequestedSize);
    for (int32_t i = 0; i < (int32_t)PendingList.size(); ++i) {
      if (PendingList[i].Size >= InRequestedSize) {
        PlacedResource resource = PendingList[i];
        PendingList.erase(PendingList.begin() + i);
        UsingPlacedResources.insert(
            std::make_pair(resource.PlacedSubResource.Get(), resource));
        return resource;
      }
    }

    return PlacedResource();
  }

  void Free(const ComPtr<ID3D12Resource>& InData);

  void AddUsingPlacedResource(const PlacedResource InPlacedResource) {
    assert(InPlacedResource.IsValid());
    {
      ScopedLock s(&Lock);
      UsingPlacedResources.insert(std::make_pair(
          InPlacedResource.PlacedSubResource.Get(), InPlacedResource));
    }
  }

  // This will be called from 'DeallocatorMultiFrameUniformBufferBlock'
  // TODO: consider remove nested smart pointers (probably need changes in
  // DeallocatorMultiFrameResource)
  void FreedFromPendingDelegateForCreatedResource(
      const std::shared_ptr<ComPtr<ID3D12Resource>>& InData) {
    Free(InData.get()->Get());
  }

  std::vector<PlacedResource>& GetPendingPlacedResources(
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
  std::map<ID3D12Resource*, PlacedResource> UsingPlacedResources;
  std::vector<PlacedResource>
      PendingPlacedResources[(int32_t)EPoolSizeType::MAX];
  std::vector<PlacedResource>
      PendingUploadPlacedResources[(int32_t)EPoolSizeType::MAX];
};

class RhiDx12 : public RHI {
  public:
  static constexpr UINT MaxFrameCount = 3;

  RhiDx12();
  virtual ~RhiDx12();

  //////////////////////////////////////////////////////////////////////////
  // 1. Device
  ComPtr<IDXGIAdapter3> Adapter;
  ComPtr<ID3D12Device5> Device;
  uint32_t              Options = 0;
  ComPtr<IDXGIFactory5> Factory;

  //////////////////////////////////////////////////////////////////////////
  // 2. Command
  CommandBufferManagerDx12* m_commandBufferManager     = nullptr;
  CommandBufferManagerDx12* CopyCommandBufferManager = nullptr;

  //////////////////////////////////////////////////////////////////////////
  // 3. Swapchain
  std::shared_ptr<SwapchainDx12> m_swapchain_
      = std::make_shared<SwapchainDx12>();
  uint32_t CurrentFrameIndex = 0;

  //////////////////////////////////////////////////////////////////////////
  // 4. Heap
  OfflineDescriptorHeapDx12 RTVDescriptorHeaps;
  OfflineDescriptorHeapDx12 DSVDescriptorHeaps;
  OfflineDescriptorHeapDx12 DescriptorHeaps;
  OfflineDescriptorHeapDx12 SamplerDescriptorHeaps;

  OnlineDescriptorManager OnlineDescriptorHeapManager;

  // 7. Create sync object
  FenceManagerDx12 m_fenceManager_;
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

  PlacedResourcePool m_placedResourcePool;

  template <typename T>
  std::shared_ptr<CreatedResource> CreateResource(
      T&&                   InDesc,
      D3D12_RESOURCE_STATES InResourceState,
      D3D12_CLEAR_VALUE*    InClearValue = nullptr) {
    assert(Device);

    if (GIsUsePlacedResource) {
      const D3D12_RESOURCE_ALLOCATION_INFO info
          = Device->GetResourceAllocationInfo(0, 1, InDesc);

      PlacedResource ReusePlacedResource
          = m_placedResourcePool.Alloc(info.SizeInBytes, false);
      if (ReusePlacedResource.IsValid()) {
        return CreatedResource::CreatedFromResourcePool(
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

          PlacedResource NewPlacedResource;
          NewPlacedResource.IsUploadResource  = false;
          NewPlacedResource.PlacedSubResource = NewResource;
          NewPlacedResource.Size              = info.SizeInBytes;
          m_placedResourcePool.AddUsingPlacedResource(NewPlacedResource);

          return CreatedResource::CreatedFromResourcePool(
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
    return CreatedResource::CreatedFromStandalone(NewResource);
  }

  template <typename T>
  std::shared_ptr<CreatedResource> CreateUploadResource(
      T&&                   InDesc,
      D3D12_RESOURCE_STATES InResourceState,
      D3D12_CLEAR_VALUE*    InClearValue = nullptr) {
    assert(Device);

    if (GIsUsePlacedResource) {
      const D3D12_RESOURCE_ALLOCATION_INFO info
          = Device->GetResourceAllocationInfo(0, 1, InDesc);

      PlacedResource ReusePlacedUploadResource
          = m_placedResourcePool.Alloc(info.SizeInBytes, true);
      if (ReusePlacedUploadResource.IsValid()) {
        return CreatedResource::CreatedFromResourcePool(
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

          PlacedResource NewPlacedResource;
          NewPlacedResource.IsUploadResource  = true;
          NewPlacedResource.PlacedSubResource = NewResource;
          NewPlacedResource.Size              = info.SizeInBytes;
          m_placedResourcePool.AddUsingPlacedResource(NewPlacedResource);

          return CreatedResource::CreatedFromResourcePool(NewResource);
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
    return CreatedResource::CreatedFromStandalone(NewResource);
  }

  //////////////////////////////////////////////////////////////////////////

  virtual bool OnHandleResized(uint32_t InWidth,
                               uint32_t InHeight,
                               bool     InIsMinimized) override;

  virtual CommandBufferDx12* BeginSingleTimeCommands() const override;
  virtual void                 EndSingleTimeCommands(
                      CommandBuffer* commandBuffer) const override;

  CommandBufferDx12* BeginSingleTimeCopyCommands() const;
  void EndSingleTimeCopyCommands(CommandBufferDx12* commandBuffer) const;

  virtual std::shared_ptr<Texture> CreateTextureFromData(
      const ImageData* InImageData) const override;

  virtual FenceManager* GetFenceManager() override { return &m_fenceManager_; }

  std::vector<RingBufferDx12*> OneFrameUniformRingBuffers;

  RingBufferDx12* GetOneFrameUniformRingBuffer() const {
    return OneFrameUniformRingBuffers[CurrentFrameIndex];
  }

  virtual ShaderBindingLayout* CreateShaderBindings(
      const ShaderBindingArray& InShaderBindingArray) const override;

  virtual SamplerStateInfo* CreateSamplerState(
      const SamplerStateInfo& initializer) const override;
  virtual RasterizationStateInfo* CreateRasterizationState(
      const RasterizationStateInfo& initializer) const override;
  virtual StencilOpStateInfo* CreateStencilOpStateInfo(
      const StencilOpStateInfo& initializer) const override;
  virtual DepthStencilStateInfo* CreateDepthStencilState(
      const DepthStencilStateInfo& initializer) const override;
  virtual BlendingStateInfo* CreateBlendingState(
      const BlendingStateInfo& initializer) const override;

  uint32_t CurrentFrameNumber
      = 0;  // FrameNumber is just Incremented frame by frame.

  virtual void IncrementFrameNumber() { ++CurrentFrameNumber; }

  virtual uint32_t GetCurrentFrameNumber() const override {
    return CurrentFrameNumber;
  }

  virtual uint32_t GetCurrentFrameIndex() const { return CurrentFrameIndex; }

  static std::unordered_map<size_t, ShaderBindingLayout*> ShaderBindingPool;
  mutable MutexRWLock ShaderBindingPoolLock;

  static TResourcePool<SamplerStateInfoDx12, MutexRWLock> SamplerStatePool;
  static TResourcePool<RasterizationStateInfoDx12, MutexRWLock>
      RasterizationStatePool;
  static TResourcePool<StencilOpStateInfoDx12, MutexRWLock>
      StencilOpStatePool;
  static TResourcePool<DepthStencilStateInfoDx12, MutexRWLock>
      DepthStencilStatePool;
  static TResourcePool<BlendingStateInfoDx12, MutexRWLock> BlendingStatePool;
  static TResourcePool<PipelineStateInfoDx12, MutexRWLock> PipelineStatePool;
  static TResourcePool<RenderPassDx12, MutexRWLock>        RenderPassPool;

  virtual bool CreateShaderInternal(
      Shader* OutShader, const ShaderInfo& shaderInfo) const override;

  virtual RenderPass* GetOrCreateRenderPass(
      const std::vector<Attachment>& colorAttachments,
      const math::Vector2Di&          offset,
      const math::Vector2Di&          extent) const override;
  virtual RenderPass* GetOrCreateRenderPass(
      const std::vector<Attachment>& colorAttachments,
      const Attachment&              depthAttachment,
      const math::Vector2Di&          offset,
      const math::Vector2Di&          extent) const override;
  virtual RenderPass* GetOrCreateRenderPass(
      const std::vector<Attachment>& colorAttachments,
      const Attachment&              depthAttachment,
      const Attachment&              colorResolveAttachment,
      const math::Vector2Di&          offset,
      const math::Vector2Di&          extent) const override;
  virtual RenderPass* GetOrCreateRenderPass(
      const RenderPassInfo& renderPassInfo,
      const math::Vector2Di& offset,
      const math::Vector2Di& extent) const override;

  virtual PipelineStateInfo* CreatePipelineStateInfo(
      const PipelineStateFixedInfo*   InPipelineStateFixed,
      const GraphicsPipelineShader     InShader,
      const VertexBufferArray&        InVertexBufferArray,
      const RenderPass*               InRenderPass,
      const ShaderBindingLayoutArray& InShaderBindingArray,
      const PushConstant*             InPushConstant,
      int32_t                          InSubpassIndex) const override;
  virtual PipelineStateInfo* CreateComputePipelineStateInfo(
      const Shader*                    shader,
      const ShaderBindingLayoutArray& InShaderBindingArray,
      const PushConstant*             pushConstant) const override;

  virtual void RemovePipelineStateInfo(size_t InHash) override;

  virtual std::shared_ptr<RenderFrameContext> BeginRenderFrame() override;
  virtual void EndRenderFrame(const std::shared_ptr<RenderFrameContext>&
                                  renderFrameContextPtr) override;

  virtual CommandBufferManagerDx12* GetCommandBufferManager() const override {
    return m_commandBufferManager;
  }

  virtual CommandBufferManagerDx12* GetCopyCommandBufferManager() const {
    return CopyCommandBufferManager;
  }

  virtual void BindGraphicsShaderBindingInstances(
      const CommandBuffer*                InCommandBuffer,
      const PipelineStateInfo*            InPiplineState,
      const ShaderBindingInstanceCombiner& InShaderBindingInstanceCombiner,
      uint32_t                             InFirstSet) const override;
  virtual void BindComputeShaderBindingInstances(
      const CommandBuffer*                InCommandBuffer,
      const PipelineStateInfo*            InPiplineState,
      const ShaderBindingInstanceCombiner& InShaderBindingInstanceCombiner,
      uint32_t                             InFirstSet) const override;
  virtual void BindRaytracingShaderBindingInstances(
      const CommandBuffer*                InCommandBuffer,
      const PipelineStateInfo*            InPiplineState,
      const ShaderBindingInstanceCombiner& InShaderBindingInstanceCombiner,
      uint32_t                             InFirstSet) const override;

  virtual std::shared_ptr<ShaderBindingInstance> CreateShaderBindingInstance(
      const ShaderBindingArray&       InShaderBindingArray,
      const ShaderBindingInstanceType InType) const override;

  virtual void DrawArrays(
      const std::shared_ptr<RenderFrameContext>& InRenderFrameContext,
      // EPrimitiveType                              type,
      int32_t                                     vertStartIndex,
      int32_t                                     vertCount) const override;
  virtual void DrawArraysInstanced(
      const std::shared_ptr<RenderFrameContext>& InRenderFrameContext,
      // EPrimitiveType                              type,
      int32_t                                     vertStartIndex,
      int32_t                                     vertCount,
      int32_t                                     instanceCount) const override;
  virtual void DrawElements(
      const std::shared_ptr<RenderFrameContext>& InRenderFrameContext,
      // EPrimitiveType                              type,
      int32_t                                     elementSize,
      int32_t                                     startIndex,
      int32_t                                     indexCount) const override;
  virtual void DrawElementsInstanced(
      const std::shared_ptr<RenderFrameContext>& InRenderFrameContext,
      // EPrimitiveType                              type,
      int32_t                                     elementSize,
      int32_t                                     startIndex,
      int32_t                                     indexCount,
      int32_t                                     instanceCount) const override;
  virtual void DrawElementsBaseVertex(
      const std::shared_ptr<RenderFrameContext>& InRenderFrameContext,
      // EPrimitiveType                              type,
      int32_t                                     elementSize,
      int32_t                                     startIndex,
      int32_t                                     indexCount,
      int32_t baseVertexIndex) const override;
  virtual void DrawElementsInstancedBaseVertex(
      const std::shared_ptr<RenderFrameContext>& InRenderFrameContext,
      // EPrimitiveType                              type,
      int32_t                                     elementSize,
      int32_t                                     startIndex,
      int32_t                                     indexCount,
      int32_t                                     baseVertexIndex,
      int32_t                                     instanceCount) const override;
  virtual void DrawIndirect(
      const std::shared_ptr<RenderFrameContext>& InRenderFrameContext,
      // EPrimitiveType                              type,
      Buffer*                                    buffer,
      int32_t                                     startIndex,
      int32_t                                     drawCount) const override;
  virtual void DrawElementsIndirect(
      const std::shared_ptr<RenderFrameContext>& InRenderFrameContext,
      // EPrimitiveType                              type,
      Buffer*                                    buffer,
      int32_t                                     startIndex,
      int32_t                                     drawCount) const override;
  virtual void DispatchCompute(
      const std::shared_ptr<RenderFrameContext>& InRenderFrameContext,
      uint32_t                                    numGroupsX,
      uint32_t                                    numGroupsY,
      uint32_t                                    numGroupsZ) const override;

  virtual void* GetWindow() const override { return m_hWnd; }

  virtual std::shared_ptr<RenderTarget> CreateRenderTarget(
      const RenderTargetInfo& info) const override;

  // Resource Barrier
  bool         TransitionLayout_Internal(CommandBuffer*       commandBuffer,
                                         ID3D12Resource*       resource,
                                         D3D12_RESOURCE_STATES srcLayout,
                                         D3D12_RESOURCE_STATES dstLayout) const;
  virtual bool TransitionLayout(CommandBuffer* commandBuffer,
                                Texture*       texture,
                                EResourceLayout newLayout) const override;
  virtual bool TransitionLayoutImmediate(
      Texture* texture, EResourceLayout newLayout) const override;
  virtual bool TransitionLayout(CommandBuffer* commandBuffer,
                                Buffer*        buffer,
                                EResourceLayout newLayout) const override;
  virtual bool TransitionLayoutImmediate(
      Buffer* buffer, EResourceLayout newLayout) const override;

  virtual void UAVBarrier(CommandBuffer* commandBuffer,
                          Texture*       texture) const override;
  virtual void UAVBarrierImmediate(Texture* texture) const override;
  virtual void UAVBarrier(CommandBuffer* commandBuffer,
                          Buffer*        buffer) const override;
  virtual void UAVBarrierImmediate(Buffer* buffer) const override;

  //////////////////////////////////////////////////////////////////////////

  virtual std::shared_ptr<Swapchain> GetSwapchain() const override {
    return m_swapchain_;
  }

  virtual SwapchainImage* GetSwapchainImage(int32_t InIndex) const override {
    return m_swapchain_->GetSwapchainImage(InIndex);
  }

  // TODO: implement
  // virtual void BeginDebugEvent(CommandBuffer*        InCommandBuffer,
  //                             const char*            InName,
  //                             const math::Vector4Df& InColor
  //                             = math::ColorGreen) const override;

  // virtual void EndDebugEvent(CommandBuffer* InCommandBuffer) const override;

  virtual void Flush() const override;
  virtual void Finish() const override;

  MutexLock MultiFrameShaderBindingInstanceLock;
  DeallocatorMultiFrameShaderBindingInstance
      m_deallocatorMultiFrameShaderBindingInstance;
  DeallocatorMultiFrameCreatedResource DeallocatorMultiFramePlacedResource;
  DeallocatorMultiFrameCreatedResource DeallocatorMultiFrameStandaloneResource;

  // Create m_buffers
  virtual std::shared_ptr<Buffer> CreateStructuredBuffer(
      uint64_t          InSize,
      uint64_t          InAlignment,
      uint64_t          InStride,
      EBufferCreateFlag InBufferCreateFlag,
      EResourceLayout   InInitialState,
      const void*       InData     = nullptr,
      uint64_t          InDataSize = 0
      /*, const wchar_t*    InResourceName = nullptr*/) const override;

  virtual std::shared_ptr<Buffer> CreateRawBuffer(
      uint64_t          InSize,
      uint64_t          InAlignment,
      EBufferCreateFlag InBufferCreateFlag,
      EResourceLayout   InInitialState,
      const void*       InData     = nullptr,
      uint64_t          InDataSize = 0
      /*, const wchar_t*    InResourceName = nullptr*/) const override;

  virtual std::shared_ptr<Buffer> CreateFormattedBuffer(
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
  virtual std::shared_ptr<VertexBuffer> CreateVertexBuffer(
      const std::shared_ptr<VertexStreamData>& streamData) const override;
  virtual std::shared_ptr<IndexBuffer> CreateIndexBuffer(
      const std::shared_ptr<IndexStreamData>& streamData) const override;
  //////////////////////////////////////////////////////////////////////////

  // Create Images
  virtual std::shared_ptr<Texture> Create2DTexture(
      uint32_t             InWidth,
      uint32_t             InHeight,
      uint32_t             InArrayLayers,
      uint32_t             InMipLevels,
      ETextureFormat       InFormat,
      ETextureCreateFlag   InTextureCreateFlag,
      EResourceLayout      InImageLayout   = EResourceLayout::UNDEFINED,
      const ImageBulkData& InImageBulkData = {},
      const RTClearValue& InClearValue    = RTClearValue::Invalid,
      const wchar_t*       InResourceName  = nullptr) const override;

  virtual std::shared_ptr<Texture> CreateCubeTexture(
      uint32_t             InWidth,
      uint32_t             InHeight,
      uint32_t             InMipLevels,
      ETextureFormat       InFormat,
      ETextureCreateFlag   InTextureCreateFlag,
      EResourceLayout      InImageLayout   = EResourceLayout::UNDEFINED,
      const ImageBulkData& InImageBulkData = {},
      const RTClearValue& InClearValue    = RTClearValue::Invalid,
      const wchar_t*       InResourceName  = nullptr) const override;
  //////////////////////////////////////////////////////////////////////////

  virtual bool IsSupportVSync() const override;

  private:
  // TODO: consider whether need in this place
  std::shared_ptr<Window> m_window_;
};

extern RhiDx12* g_rhi_dx12;

}  // namespace game_engine

#endif  // GAME_ENGINE_RHI_DX12_H