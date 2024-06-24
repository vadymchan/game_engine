#ifndef GAME_ENGINE_RHI_H
#define GAME_ENGINE_RHI_H

#include "file_loader/image_file_loader.h"  // TODO: may case circular dependency
#include "gfx/renderer/material.h"
#include "gfx/rhi/buffer.h"
#include "gfx/rhi/frame_buffer.h"
#include "gfx/rhi/i_uniform_buffer_block.h"
#include "gfx/rhi/lock.h"
#include "gfx/rhi/memory_pool.h"
#include "gfx/rhi/name.h"
#include "gfx/rhi/pipeline_state_info.h"
#include "gfx/rhi/render_pass.h"
#include "gfx/rhi/resource_pool.h"
#include "gfx/rhi/rhi_type.h"
#include "gfx/rhi/semaphore_manager.h"
#include "gfx/rhi/shader_binding_instance_combiner.h"
#include "gfx/rhi/texture.h"
#include "platform/common/window.h"
#include "utils/math/math_util.h"

#include <cstdint>
#include <memory>

namespace game_engine {
// TODO: move in other place
extern std::int32_t GMaxCheckCountForRealTimeShaderUpdate;
extern std::int32_t GSleepMSForRealTimeShaderUpdate;
extern bool         GUseRealTimeShaderUpdate;

extern std::shared_ptr<jTexture>  GWhiteTexture;
extern std::shared_ptr<jTexture>  GBlackTexture;
extern std::shared_ptr<jTexture>  GWhiteCubeTexture;
extern std::shared_ptr<jTexture>  GNormalTexture;
extern std::shared_ptr<jMaterial> GDefaultMaterial;

struct Shader;
struct ShaderInfo;

// struct ImageData; // #include "file_loader/image_file_loader.h"

class jRHI {
  public:
  jRHI();

  virtual ~jRHI() {}

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

  virtual Name GetRHIName() { return Name::Invalid; }

  virtual bool init(const std::shared_ptr<Window>& window);
  virtual void OnInitRHI();
  virtual void release();

  virtual void* GetWindow() const { return nullptr; }

  virtual jSamplerStateInfo* CreateSamplerState(
      const jSamplerStateInfo& info) const {
    return nullptr;
  }

  virtual void ReleaseSamplerState(jSamplerStateInfo* samplerState) const {}

  virtual void BindSamplerState(std::int32_t             index,
                                const jSamplerStateInfo* samplerState) const {}

  virtual void SetClear(ERenderBufferType typeBit) const {}

  virtual void SetClearColor(float r, float g, float b, float a) const {}

  virtual void SetClearColor(math::Vector4Df rgba) const {}

  virtual void SetClearBuffer(ERenderBufferType typeBit,
                              const float*      value,
                              std::int32_t      bufferIndex) const {}

  virtual void SetClearBuffer(ERenderBufferType   typeBit,
                              const std::int32_t* value,
                              std::int32_t        bufferIndex) const {}

  virtual void SetFrameBuffer(const jFrameBuffer* rt,
                              std::int32_t        index = 0,
                              bool                mrt   = false) const {}

  virtual void SetDrawBuffers(
      const std::initializer_list<EDrawBufferType>& list) const {}

  virtual void SetTextureFilter(ETextureType         type,
                                std::int32_t         sampleCount,
                                ETextureFilterTarget target,
                                ETextureFilter       filter) const {}

  virtual void SetTextureWrap(int flag) const {}

  virtual void SetTexture(std::int32_t index, const jTexture* texture) const {}

  virtual void DrawArrays(
      const std::shared_ptr<jRenderFrameContext>& InRenderFrameContext,
      // EPrimitiveType                              type,
      std::int32_t                                vertStartIndex,
      std::int32_t                                vertCount) const {}

  virtual void DrawArraysInstanced(
      const std::shared_ptr<jRenderFrameContext>& InRenderFrameContext,
      // EPrimitiveType                              type,
      std::int32_t                                vertStartIndex,
      std::int32_t                                vertCount,
      std::int32_t                                instanceCount) const {}

  virtual void DrawElements(
      const std::shared_ptr<jRenderFrameContext>& InRenderFrameContext,
      // EPrimitiveType                              type,
      std::int32_t                                elementSize,
      std::int32_t                                startIndex,
      std::int32_t                                indexCount) const {}

  virtual void DrawElementsInstanced(
      const std::shared_ptr<jRenderFrameContext>& InRenderFrameContext,
      // EPrimitiveType                              type,
      std::int32_t                                elementSize,
      std::int32_t                                startIndex,
      std::int32_t                                indexCount,
      std::int32_t                                instanceCount) const {}

  virtual void DrawElementsBaseVertex(
      const std::shared_ptr<jRenderFrameContext>& InRenderFrameContext,
      // EPrimitiveType                              type,
      std::int32_t                                elementSize,
      std::int32_t                                startIndex,
      std::int32_t                                indexCount,
      std::int32_t                                baseVertexIndex) const {}

  virtual void DrawElementsInstancedBaseVertex(
      const std::shared_ptr<jRenderFrameContext>& InRenderFrameContext,
      // EPrimitiveType                              type,
      std::int32_t                                elementSize,
      std::int32_t                                startIndex,
      std::int32_t                                indexCount,
      std::int32_t                                baseVertexIndex,
      std::int32_t                                instanceCount) const {}

  virtual void DrawIndirect(
      const std::shared_ptr<jRenderFrameContext>& InRenderFrameContext,
      // EPrimitiveType                              type,
      jBuffer*                                    buffer,
      std::int32_t                                startIndex,
      std::int32_t                                drawCount) const {}

  virtual void DrawElementsIndirect(
      const std::shared_ptr<jRenderFrameContext>& InRenderFrameContext,
      // EPrimitiveType                              type,
      jBuffer*                                    buffer,
      std::int32_t                                startIndex,
      std::int32_t                                drawCount) const {}

  virtual void DispatchCompute(
      const std::shared_ptr<jRenderFrameContext>& InRenderFrameContext,
      std::uint32_t                               numGroupsX,
      std::uint32_t                               numGroupsY,
      std::uint32_t                               numGroupsZ) const {}

  // TODO: implement in future iterations
  // virtual void DispatchRay(
  //    const std::shared_ptr<RenderFrameContext>& InRenderFrameContext,
  //    const RaytracingDispatchData&              InDispatchData) const {}

  virtual void EnableDepthBias(bool         enable,
                               EPolygonMode polygonMode
                               = EPolygonMode::FILL) const {}

  virtual void SetDepthBias(float constant, float slope) const {}

  virtual bool CreateShaderInternal(Shader*           OutShader,
                                    const ShaderInfo& shaderInfo) const {
    return false;
  }

  virtual void ReleaseShader(Shader* shader) const {}

  virtual void SetViewport(std::int32_t x,
                           std::int32_t y,
                           std::int32_t width,
                           std::int32_t height) const {}

  virtual void SetViewport(const Viewport& viewport) const {}

  virtual void SetViewportIndexed(
      std::int32_t index, float x, float y, float width, float height) const {}

  virtual void SetViewportIndexed(std::int32_t    index,
                                  const Viewport& viewport) const {}

  virtual void SetViewportIndexedArray(std::int32_t    startIndex,
                                       std::int32_t    count,
                                       const Viewport* viewports) const {}

  virtual bool SetUniformbuffer(const Name&           name,
                                const math::Matrix4d& InData,
                                const Shader*         InShader) const {
    return false;
  }

  virtual bool SetUniformbuffer(const Name&   name,
                                const int     InData,
                                const Shader* InShader) const {
    return false;
  }

  virtual bool SetUniformbuffer(const Name&         name,
                                const std::uint32_t InData,
                                const Shader*       InShader) const {
    return false;
  }

  inline virtual bool SetUniformbuffer(Name          name,
                                       const bool    InData,
                                       const Shader* InShader) const {
    return SetUniformbuffer(name, (std::int32_t)InData, InShader);
  }

  virtual bool SetUniformbuffer(const Name&   name,
                                const float   InData,
                                const Shader* InShader) const {
    return false;
  }

  virtual bool SetUniformbuffer(const Name&            name,
                                const math::Vector2Df& InData,
                                const Shader*          InShader) const {
    return false;
  }

  virtual bool SetUniformbuffer(const Name&              name,
                                const math::VectorNf<1>& InData,
                                const Shader*            InShader) const {
    return false;
  }

  virtual bool SetUniformbuffer(const Name&            name,
                                const math::Vector4Df& InData,
                                const Shader*          InShader) const {
    return false;
  }

  virtual bool SetUniformbuffer(const Name&            name,
                                const math::Vector2Di& InData,
                                const Shader*          InShader) const {
    return false;
  }

  virtual bool SetUniformbuffer(const Name&            name,
                                const math::Vector3Di& InData,
                                const Shader*          InShader) const {
    return false;
  }

  virtual bool SetUniformbuffer(const Name&            name,
                                const math::Vector4Di& InData,
                                const Shader*          InShader) const {
    return false;
  }

  virtual bool GetUniformbuffer(math::Matrix4d& outResult,
                                const Name&     name,
                                const Shader*   shader) const {
    return false;
  }

  virtual bool GetUniformbuffer(int&          outResult,
                                const Name&   name,
                                const Shader* shader) const {
    return false;
  }

  virtual bool GetUniformbuffer(std::uint32_t& outResult,
                                const Name&    name,
                                const Shader*  shader) const {
    return false;
  }

  virtual bool GetUniformbuffer(float&        outResult,
                                const Name&   name,
                                const Shader* shader) const {
    return false;
  }

  virtual bool GetUniformbuffer(math::Vector2Df& outResult,
                                const Name&      name,
                                const Shader*    shader) const {
    return false;
  }

  virtual bool GetUniformbuffer(math::VectorNf<1>& outResult,
                                const Name&        name,
                                const Shader*      shader) const {
    return false;
  }

  virtual bool GetUniformbuffer(math::Vector4Df& outResult,
                                const Name&      name,
                                const Shader*    shader) const {
    return false;
  }

  virtual bool GetUniformbuffer(math::Vector2Di& outResult,
                                const Name&      name,
                                const Shader*    shader) const {
    return false;
  }

  virtual bool GetUniformbuffer(math::Vector3Di& outResult,
                                const Name&      name,
                                const Shader*    shader) const {
    return false;
  }

  virtual bool GetUniformbuffer(math::Vector4Di& outResult,
                                const Name&      name,
                                const Shader*    shader) const {
    return false;
  }

  virtual jTexture* CreateNullTexture() const { return nullptr; }

  virtual std::shared_ptr<jTexture> CreateTextureFromData(
      const ImageData* InImageData) const {
    return nullptr;
  }

  virtual jTexture* CreateCubeTextureFromData(std::vector<void*> faces,
                                              std::int32_t       width,
                                              std::int32_t       height,
                                              bool               sRGB,
                                              ETextureFormat     textureFormat
                                              = ETextureFormat::RGBA8,
                                              bool createMipmap = false) const {
    return nullptr;
  }

  virtual jFrameBuffer* CreateFrameBuffer(const jFrameBufferInfo& info) const {
    return nullptr;
  }

  virtual std::shared_ptr<jRenderTarget> CreateRenderTarget(
      const jRenderTargetInfo& info) const {
    return nullptr;
  }

  virtual void EnableDepthTest(bool enable) const {}

  virtual void EnableBlend(bool enable) const {}

  virtual void SetBlendFunc(EBlendFactor src, EBlendFactor dest) const {}

  virtual void SetBlendFuncRT(EBlendFactor src,
                              EBlendFactor dest,
                              std::int32_t rtIndex = 0) const {}

  virtual void SetBlendEquation(EBlendOp func) const {}

  virtual void SetBlendEquation(EBlendOp func, std::int32_t rtIndex) const {}

  virtual void SetBlendColor(float r, float g, float b, float a) const {}

  virtual void EnableStencil(bool enable) const {}

  virtual void SetStencilOpSeparate(EFace      face,
                                    EStencilOp sFail,
                                    EStencilOp dpFail,
                                    EStencilOp dpPass) const {}

  virtual void SetStencilFunc(ECompareOp    func,
                              std::int32_t  ref,
                              std::uint32_t mask) const {}

  virtual void SetDepthFunc(ECompareOp func) const {}

  virtual void SetDepthMask(bool enable) const {}

  virtual void SetColorMask(bool r, bool g, bool b, bool a) const {}

  virtual void EnableSRGB(bool enable) const {}

  virtual void EnableDepthClip(bool enable) const {}

  virtual void BeginDebugEvent(const char* name) const {}

  virtual void EndDebugEvent() const {}

  virtual void BeginDebugEvent(jCommandBuffer*        InCommandBuffer,
                               const char*            InName,
                               const math::Vector4Df& InColor
                               = math::ColorGreen) const {}

  virtual void EndDebugEvent(jCommandBuffer* InCommandBuffer) const {}

  virtual void GenerateMips(const jTexture* texture) const {}

  virtual void EnableWireframe(bool enable) const {}

  virtual void SetImageTexture(std::int32_t            index,
                               const jTexture*         texture,
                               EImageTextureAccessType type) const {}

  virtual void SetPolygonMode(EFace        face,
                              EPolygonMode mode = EPolygonMode::FILL) {}

  virtual void EnableRasterizerDiscard(bool enable) const {}

  virtual void SetTextureMipmapLevelLimit(ETextureType type,
                                          std::int32_t sampleCount,
                                          std::int32_t baseLevel,
                                          std::int32_t maxLevel) const {}

  virtual void EnableMultisample(bool enable) const {}

  virtual void SetCubeMapSeamless(bool enable) const {}

  virtual void SetLineWidth(float width) const {}

  virtual void Flush() const {}

  virtual void Finish() const {}

  virtual std::shared_ptr<jRenderFrameContext> BeginRenderFrame() {
    return nullptr;
  }

  virtual void EndRenderFrame(
      const std::shared_ptr<jRenderFrameContext>& renderFrameContextPtr) {}

  virtual void QueueSubmit(
      const std::shared_ptr<jRenderFrameContext>& renderFrameContextPtr,
      class jSemaphore*                           InSignalSemaphore) {}

  virtual jRasterizationStateInfo* CreateRasterizationState(
      const jRasterizationStateInfo& initializer) const {
    return nullptr;
  }

  virtual jStencilOpStateInfo* CreateStencilOpStateInfo(
      const jStencilOpStateInfo& initializer) const {
    return nullptr;
  }

  virtual jDepthStencilStateInfo* CreateDepthStencilState(
      const jDepthStencilStateInfo& initializer) const {
    return nullptr;
  }

  virtual jBlendingStateInfo* CreateBlendingState(
      const jBlendingStateInfo& initializer) const {
    return nullptr;
  }

  virtual jPipelineStateInfo* CreatePipelineStateInfo(
      const jPipelineStateFixedInfo*   pipelineStateFixed,
      const GraphicsPipelineShader     shader,
      const jVertexBufferArray&        InVertexBufferArray,
      const jRenderPass*               renderPass,
      const jShaderBindingLayoutArray& InShaderBindingArray,
      const jPushConstant*             InPushConstant,
      std::int32_t                     InSubpassIndex) const {
    return nullptr;
  }

  virtual jPipelineStateInfo* CreateComputePipelineStateInfo(
      const Shader*                    shader,
      const jShaderBindingLayoutArray& InShaderBindingArray,
      const jPushConstant*             pushConstant) const {
    return nullptr;
  }

  // TODO: implement
  // virtual PipelineStateInfo* CreateRaytracingPipelineStateInfo(
  //    const std::vector<RaytracingPipelineShader>& InShaders,
  //    const RaytracingPipelineData&                InRaytracingData,
  //    const ShaderBindingLayoutArray&              InShaderBindingArray,
  //    const PushConstant*                          pushConstant) const {
  //  return nullptr;
  //}

  virtual void RemovePipelineStateInfo(size_t InHash) {}

  virtual jShaderBindingLayout* CreateShaderBindings(
      const jShaderBindingArray& InShaderBindingArray) const {
    assert(0);
    return nullptr;
  }

  virtual std::shared_ptr<jShaderBindingInstance> CreateShaderBindingInstance(
      const jShaderBindingArray&       InShaderBindingArray,
      const jShaderBindingInstanceType InType) const {
    assert(0);
    return nullptr;
  }

  virtual jRenderPass* GetOrCreateRenderPass(
      const std::vector<jAttachment>& colorAttachments,
      const math::Vector2Di&          offset,
      const math::Vector2Di&          extent) const {
    return nullptr;
  }

  virtual jRenderPass* GetOrCreateRenderPass(
      const std::vector<jAttachment>& colorAttachments,
      const jAttachment&              depthAttachment,
      const math::Vector2Di&          offset,
      const math::Vector2Di&          extent) const {
    return nullptr;
  }

  virtual jRenderPass* GetOrCreateRenderPass(
      const std::vector<jAttachment>& colorAttachments,
      const jAttachment&              depthAttachment,
      const jAttachment&              colorResolveAttachment,
      const math::Vector2Di&          offset,
      const math::Vector2Di&          extent) const {
    return nullptr;
  }

  virtual jRenderPass* GetOrCreateRenderPass(
      const jRenderPassInfo& renderPassInfo,
      const math::Vector2Di& offset,
      const math::Vector2Di& extent) const {
    return nullptr;
  }

  virtual jCommandBufferManager* GetCommandBufferManager() const {
    return nullptr;
  }

  virtual EMSAASamples GetSelectedMSAASamples() const {
    return EMSAASamples::COUNT_1;
  }

  // ResourceBarrier
  virtual bool TransitionLayout(jCommandBuffer* commandBuffer,
                                jTexture*       texture,
                                EResourceLayout newLayout) const {
    return true;
  }

  virtual bool TransitionLayoutImmediate(jTexture*       texture,
                                         EResourceLayout newLayout) const {
    return true;
  }

  virtual bool TransitionLayout(jCommandBuffer* commandBuffer,
                                jBuffer*        buffer,
                                EResourceLayout newLayout) const {
    return true;
  }

  virtual bool TransitionLayoutImmediate(jBuffer*        buffer,
                                         EResourceLayout newLayout) const {
    return true;
  }

  virtual void UAVBarrier(jCommandBuffer* commandBuffer,
                          jTexture*       texture) const {}

  virtual void UAVBarrierImmediate(jTexture* texture) const {}

  virtual void UAVBarrier(jCommandBuffer* commandBuffer,
                          jBuffer*        buffer) const {}

  virtual void UAVBarrierImmediate(jBuffer* buffer) const {}

  //////////////////////////////////////////////////////////////////////////

  virtual std::shared_ptr<jSwapchain> GetSwapchain() const { return nullptr; }

  virtual class jSwapchainImage* GetSwapchainImage(std::int32_t InIndex) const {
    return nullptr;
  }

  virtual void RecreateSwapChain() {}

  virtual std::uint32_t GetMaxSwapchainCount() const { return 0; }

  virtual void BindShadingRateImage(jCommandBuffer* commandBuffer,
                                    jTexture*       vrstexture) const {}

  virtual jMemoryPool* GetMemoryPool() const { return nullptr; }

  virtual void NextSubpass(const jCommandBuffer* commandBuffer) const {}

  virtual void BindGraphicsShaderBindingInstances(
      const jCommandBuffer*                InCommandBuffer,
      const jPipelineStateInfo*            InPiplineState,
      const ShaderBindingInstanceCombiner& InShaderBindingInstanceCombiner,
      std::uint32_t                        InFirstSet) const {}

  virtual void BindComputeShaderBindingInstances(
      const jCommandBuffer*                InCommandBuffer,
      const jPipelineStateInfo*            InPiplineState,
      const ShaderBindingInstanceCombiner& InShaderBindingInstanceCombiner,
      std::uint32_t                        InFirstSet) const {}

  virtual void BindRaytracingShaderBindingInstances(
      const jCommandBuffer*                InCommandBuffer,
      const jPipelineStateInfo*            InPiplineState,
      const ShaderBindingInstanceCombiner& InShaderBindingInstanceCombiner,
      std::uint32_t                        InFirstSet) const {}

  virtual jFenceManager* GetFenceManager() { return nullptr; }

  virtual jSemaphoreManager* GetSemaphoreManager() { return nullptr; }

  virtual std::uint32_t GetCurrentFrameIndex() const { return 0; }

  virtual std::uint32_t GetCurrentFrameNumber() const { return 0; }

  virtual void IncrementFrameNumber() {}

  virtual bool IsSupportVSync() const { return false; }

  virtual bool OnHandleResized(std::uint32_t InWidth,
                               std::uint32_t InHeight,
                               bool          InIsMinimized) {
    return false;
  }

  // virtual RaytracingScene* CreateRaytracingScene() const { return nullptr; }

  virtual jCommandBuffer* BeginSingleTimeCommands() const { return nullptr; }

  virtual void EndSingleTimeCommands(jCommandBuffer* commandBuffer) const {}

  // RaytracingScene* raytracingScene = nullptr;

  // CreateBuffers
  virtual std::shared_ptr<jBuffer> CreateStructuredBuffer(
      std::uint64_t     InSize,
      std::uint64_t     InAlignment,
      std::uint64_t     InStride,
      EBufferCreateFlag InBufferCreateFlag,
      EResourceLayout   InInitialState,
      const void*       InData     = nullptr,
      std::uint64_t     InDataSize = 0) const {
    return nullptr;
  }

  virtual std::shared_ptr<jBuffer> CreateRawBuffer(
      std::uint64_t     InSize,
      std::uint64_t     InAlignment,
      EBufferCreateFlag InBufferCreateFlag,
      EResourceLayout   InInitialState,
      const void*       InData     = nullptr,
      std::uint64_t     InDataSize = 0) const {
    return nullptr;
  }

  virtual std::shared_ptr<jBuffer> CreateFormattedBuffer(
      std::uint64_t     InSize,
      std::uint64_t     InAlignment,
      ETextureFormat    InFormat,
      EBufferCreateFlag InBufferCreateFlag,
      EResourceLayout   InInitialState,
      const void*       InData     = nullptr,
      std::uint64_t     InDataSize = 0) const {
    return nullptr;
  }

  virtual std::shared_ptr<IUniformBufferBlock> CreateUniformBufferBlock(
      Name InName, LifeTimeType InLifeTimeType, size_t InSize = 0) const {
    return nullptr;
  }

  virtual std::shared_ptr<jVertexBuffer> CreateVertexBuffer(
      const std::shared_ptr<VertexStreamData>& streamData) const {
    return nullptr;
  }

  virtual std::shared_ptr<jIndexBuffer> CreateIndexBuffer(
      const std::shared_ptr<IndexStreamData>& streamData) const {
    return nullptr;
  }

  template <typename T = jBuffer>
  inline std::shared_ptr<T> CreateStructuredBuffer(
      std::uint64_t     InSize,
      std::uint64_t     InAlignment,
      std::uint64_t     InStride,
      EBufferCreateFlag InBufferCreateFlag,
      EResourceLayout   InInitialState,
      const void*       InData     = nullptr,
      std::uint64_t     InDataSize = 0) const {
    return std::static_pointer_cast<T>(
        CreateStructuredBuffer(InSize,
                               InAlignment,
                               InStride,
                               InBufferCreateFlag,
                               InInitialState,
                               InData,
                               InDataSize));
  }

  template <typename T = jBuffer>
  inline std::shared_ptr<T> CreateRawBuffer(
      std::uint64_t     InSize,
      std::uint64_t     InAlignment,
      EBufferCreateFlag InBufferCreateFlag,
      EResourceLayout   InInitialState,
      const void*       InData     = nullptr,
      std::uint64_t     InDataSize = 0) const {
    return std::static_pointer_cast<T>(CreateRawBuffer(InSize,
                                                       InAlignment,
                                                       InBufferCreateFlag,
                                                       InInitialState,
                                                       InData,
                                                       InDataSize));
  }

  template <typename T = jBuffer>
  inline std::shared_ptr<T> CreateFormattedBuffer(
      std::uint64_t     InSize,
      std::uint64_t     InAlignment,
      ETextureFormat    InFormat,
      EBufferCreateFlag InBufferCreateFlag,
      EResourceLayout   InInitialState,
      const void*       InData     = nullptr,
      std::uint64_t     InDataSize = 0) const {
    return std::static_pointer_cast<T>(CreateFormattedBuffer(InSize,
                                                             InAlignment,
                                                             InFormat,
                                                             InBufferCreateFlag,
                                                             InInitialState,
                                                             InData,
                                                             InDataSize));
  }

  template <typename T = IUniformBufferBlock>
  inline std::shared_ptr<T> CreateUniformBufferBlock(
      Name InName, LifeTimeType InLifeTimeType, size_t InSize = 0) const {
    return std::static_pointer_cast<T>(
        CreateUniformBufferBlock(InName, InLifeTimeType, InSize));
  }

  //////////////////////////////////////////////////////////////////////////

  // Create Images
  virtual std::shared_ptr<jTexture> Create2DTexture(
      std::uint32_t        InWidth,
      std::uint32_t        InHeight,
      std::uint32_t        InArrayLayers,
      std::uint32_t        InMipLevels,
      ETextureFormat       InFormat,
      ETextureCreateFlag   InTextureCreateFlag,
      EResourceLayout      InImageLayout   = EResourceLayout::UNDEFINED,
      const ImageBulkData& InImageBulkData = {},
      const jRTClearValue& InClearValue    = jRTClearValue::Invalid,
      const wchar_t*       InResourceName  = nullptr) const {
    return nullptr;
  }

  virtual std::shared_ptr<jTexture> CreateCubeTexture(
      std::uint32_t        InWidth,
      std::uint32_t        InHeight,
      std::uint32_t        InMipLevels,
      ETextureFormat       InFormat,
      ETextureCreateFlag   InTextureCreateFlag,
      EResourceLayout      InImageLayout   = EResourceLayout::UNDEFINED,
      const ImageBulkData& InImageBulkData = {},
      const jRTClearValue& InClearValue    = jRTClearValue::Invalid,
      const wchar_t*       InResourceName  = nullptr) const {
    return nullptr;
  }

  template <typename T>
  std::shared_ptr<T> Create2DTexture(
      std::uint32_t        InWidth,
      std::uint32_t        InHeight,
      std::uint32_t        InArrayLayers,
      std::uint32_t        InMipLevels,
      ETextureFormat       InFormat,
      ETextureCreateFlag   InTextureCreateFlag,
      EResourceLayout      InImageLayout   = EResourceLayout::UNDEFINED,
      const ImageBulkData& InImageCopyData = {},
      const jRTClearValue& InClearValue    = jRTClearValue::Invalid,
      const wchar_t*       InResourceName  = nullptr) const {
    return std::static_pointer_cast<T>(Create2DTexture(InWidth,
                                                       InHeight,
                                                       InArrayLayers,
                                                       InMipLevels,
                                                       InFormat,
                                                       InTextureCreateFlag,
                                                       InImageLayout,
                                                       InImageCopyData,
                                                       InClearValue,
                                                       InResourceName));
  }

  template <typename T>
  std::shared_ptr<T> CreateCubeTexture(
      std::uint32_t        InWidth,
      std::uint32_t        InHeight,
      std::uint32_t        InMipLevels,
      ETextureFormat       InFormat,
      ETextureCreateFlag   InTextureCreateFlag,
      EResourceLayout      InImageLayout   = EResourceLayout::UNDEFINED,
      const ImageBulkData& InImageCopyData = {},
      const jRTClearValue& InClearValue    = jRTClearValue::Invalid,
      const wchar_t*       InResourceName  = nullptr) const {
    return std::static_pointer_cast<T>(CreateCubeTexture(InWidth,
                                                         InHeight,
                                                         InMipLevels,
                                                         InFormat,
                                                         InTextureCreateFlag,
                                                         InImageLayout,
                                                         InImageCopyData,
                                                         InClearValue,
                                                         InResourceName));
  }

  //////////////////////////////////////////////////////////////////////////
};

extern jRHI* g_rhi;

// TODO: consider move pipeline state info templated files to another file
// (current problem - g_rhi dependecty so not possible to move in
// pipeline_state_info file)

/**
 * \brief Constructs a SamplerStateInfo object with configurable parameters
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
struct TSamplerStateInfo {
  static jSamplerStateInfo* Create(math::Vector4Df BorderColor
                                   = math::Vector4Df(0.0f, 0.0f, 0.0f, 1.0f)) {
    static jSamplerStateInfo* CachedInfo = nullptr;
    if (CachedInfo) {
      return CachedInfo;
    }

    jSamplerStateInfo initializer;
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
    CachedInfo = g_rhi->CreateSamplerState(initializer);
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
struct TRasterizationStateInfo {
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
  static jRasterizationStateInfo* Create(
      std::optional<EMSAASamples> sampleCountOpt = std::nullopt) {
    static jRasterizationStateInfo* CachedInfo = nullptr;
    if (CachedInfo) {
      return CachedInfo;
    }

    jRasterizationStateInfo initializer;
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
    CachedInfo = g_rhi->CreateRasterizationState(initializer);
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
  static jDepthStencilStateInfo* Create(jStencilOpStateInfo* Front = nullptr,
                                        jStencilOpStateInfo* Back  = nullptr) {
    static jDepthStencilStateInfo* CachedInfo = nullptr;
    if (CachedInfo) {
      return CachedInfo;
    }

    jDepthStencilStateInfo initializer;
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
    CachedInfo = g_rhi->CreateDepthStencilState(initializer);
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
  static jBlendingStateInfo* Create() {
    static jBlendingStateInfo* CachedInfo = nullptr;
    if (CachedInfo) {
      return CachedInfo;
    }

    jBlendingStateInfo initializer;
    initializer.BlendEnable    = TBlendEnable;
    initializer.Src            = TSrc;
    initializer.Dest           = TDest;
    initializer.BlendOp        = TBlendOp;
    initializer.SrcAlpha       = TSrcAlpha;
    initializer.DestAlpha      = TDestAlpha;
    initializer.AlphaBlendOp   = TAlphaBlendOp;
    initializer.ColorWriteMask = TColorWriteMask;
    initializer.GetHash();
    CachedInfo = g_rhi->CreateBlendingState(initializer);
    return CachedInfo;
  }
};

}  // namespace game_engine

#endif  // GAME_ENGINE_RHI_H