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
extern std::int32_t g_maxCheckCountForRealTimeShaderUpdate;
extern std::int32_t g_sleepMSForRealTimeShaderUpdate;
extern bool         g_useRealTimeShaderUpdate;

extern std::shared_ptr<Texture>  g_whiteTexture;
extern std::shared_ptr<Texture>  g_blackTexture;
extern std::shared_ptr<Texture>  g_whiteCubeTexture;
extern std::shared_ptr<Texture>  g_normalTexture;
extern std::shared_ptr<Material> g_defaultMaterial;

struct Shader;
struct ShaderInfo;

// struct ImageData; // #include "file_loader/image_file_loader.h"

// TODO:
// - consider whether to name RHI or Rhi
// - consider implementing as singleton
class RHI {
  public:
  RHI();

  virtual ~RHI() {}

  // BEGIN: shader related functions and variables
  // =================================================================

  static TResourcePool<Shader, MutexRWLock> s_shaderPool;

  template <typename T = Shader>
  T* CreateShader(const ShaderInfo& shaderInfo) const {
    return (T*)s_shaderPool.GetOrCreate<ShaderInfo, T>(shaderInfo);
  }

  void AddShader(const ShaderInfo& shaderInfo, Shader* shader) {
    return s_shaderPool.Add(shaderInfo, shader);
  }

  void ReleaseShader(const ShaderInfo& shaderInfo) {
    s_shaderPool.Release(shaderInfo);
  }

  std::vector<Shader*> GetAllShaders() {
    std::vector<Shader*> Out;
    s_shaderPool.GetAllResource(Out);
    return Out;
  }

  // END: shader related functions and variables
  // =================================================================

  virtual Name GetRHIName() { return Name::s_kInvalid; }

  virtual bool init(const std::shared_ptr<Window>& window);
  virtual void OnInitRHI();
  virtual void release();

  virtual void* GetWindow() const { return nullptr; }

  virtual SamplerStateInfo* CreateSamplerState(
      const SamplerStateInfo& info) const {
    return nullptr;
  }

  virtual void ReleaseSamplerState(SamplerStateInfo* samplerState) const {}

  virtual void BindSamplerState(std::int32_t            index,
                                const SamplerStateInfo* samplerState) const {}

  virtual void SetClear(ERenderBufferType typeBit) const {}

  virtual void SetClearColor(float r, float g, float b, float a) const {}

  virtual void SetClearColor(math::Vector4Df rgba) const {}

  virtual void SetClearBuffer(ERenderBufferType typeBit,
                              const float*      value,
                              std::int32_t      bufferIndex) const {}

  virtual void SetClearBuffer(ERenderBufferType   typeBit,
                              const std::int32_t* value,
                              std::int32_t        bufferIndex) const {}

  virtual void SetFrameBuffer(const FrameBuffer* rt,
                              std::int32_t       index = 0,
                              bool               mrt   = false) const {}

  virtual void SetDrawBuffers(
      const std::initializer_list<EDrawBufferType>& list) const {}

  virtual void SetTextureFilter(ETextureType         type,
                                std::int32_t         sampleCount,
                                ETextureFilterTarget target,
                                ETextureFilter       filter) const {}

  virtual void SetTextureWrap(int flag) const {}

  virtual void SetTexture(std::int32_t index, const Texture* texture) const {}

  virtual void DrawArrays(
      const std::shared_ptr<RenderFrameContext>& renderFrameContext,
      // EPrimitiveType                              type,
      std::int32_t                               vertStartIndex,
      std::int32_t                               vertCount) const {}

  virtual void DrawArraysInstanced(
      const std::shared_ptr<RenderFrameContext>& renderFrameContext,
      // EPrimitiveType                              type,
      std::int32_t                               vertStartIndex,
      std::int32_t                               vertCount,
      std::int32_t                               instanceCount) const {}

  virtual void DrawElements(
      const std::shared_ptr<RenderFrameContext>& renderFrameContext,
      // EPrimitiveType                              type,
      std::int32_t                               elementSize,
      std::int32_t                               startIndex,
      std::int32_t                               indexCount) const {}

  virtual void DrawElementsInstanced(
      const std::shared_ptr<RenderFrameContext>& renderFrameContext,
      // EPrimitiveType                              type,
      std::int32_t                               elementSize,
      std::int32_t                               startIndex,
      std::int32_t                               indexCount,
      std::int32_t                               instanceCount) const {}

  virtual void DrawElementsBaseVertex(
      const std::shared_ptr<RenderFrameContext>& renderFrameContext,
      // EPrimitiveType                              type,
      std::int32_t                               elementSize,
      std::int32_t                               startIndex,
      std::int32_t                               indexCount,
      std::int32_t                               baseVertexIndex) const {}

  virtual void DrawElementsInstancedBaseVertex(
      const std::shared_ptr<RenderFrameContext>& renderFrameContext,
      // EPrimitiveType                              type,
      std::int32_t                               elementSize,
      std::int32_t                               startIndex,
      std::int32_t                               indexCount,
      std::int32_t                               baseVertexIndex,
      std::int32_t                               instanceCount) const {}

  virtual void DrawIndirect(
      const std::shared_ptr<RenderFrameContext>& renderFrameContext,
      // EPrimitiveType                              type,
      Buffer*                                    buffer,
      std::int32_t                               startIndex,
      std::int32_t                               drawCount) const {}

  virtual void DrawElementsIndirect(
      const std::shared_ptr<RenderFrameContext>& renderFrameContext,
      // EPrimitiveType                              type,
      Buffer*                                    buffer,
      std::int32_t                               startIndex,
      std::int32_t                               drawCount) const {}

  virtual void DispatchCompute(
      const std::shared_ptr<RenderFrameContext>& renderFrameContext,
      std::uint32_t                              numGroupsX,
      std::uint32_t                              numGroupsY,
      std::uint32_t                              numGroupsZ) const {}

  // TODO: implement in future iterations
  // virtual void DispatchRay(
  //    const std::shared_ptr<RenderFrameContext>& renderFrameContext,
  //    const RaytracingDispatchData&              dispatchData) const {}

  virtual void EnableDepthBias(bool         enable,
                               EPolygonMode polygonMode
                               = EPolygonMode::FILL) const {}

  virtual void SetDepthBias(float constant, float slope) const {}

  virtual bool CreateShaderInternal(Shader*           shader,
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
                                const math::Matrix4d& data,
                                const Shader*         shader) const {
    return false;
  }

  virtual bool SetUniformbuffer(const Name&   name,
                                const int     data,
                                const Shader* shader) const {
    return false;
  }

  virtual bool SetUniformbuffer(const Name&         name,
                                const std::uint32_t data,
                                const Shader*       shader) const {
    return false;
  }

  inline virtual bool SetUniformbuffer(Name          name,
                                       const bool    data,
                                       const Shader* shader) const {
    return SetUniformbuffer(name, (std::int32_t)data, shader);
  }

  virtual bool SetUniformbuffer(const Name&   name,
                                const float   data,
                                const Shader* shader) const {
    return false;
  }

  virtual bool SetUniformbuffer(const Name&            name,
                                const math::Vector2Df& data,
                                const Shader*          shader) const {
    return false;
  }

  virtual bool SetUniformbuffer(const Name&              name,
                                const math::VectorNf<1>& data,
                                const Shader*            shader) const {
    return false;
  }

  virtual bool SetUniformbuffer(const Name&            name,
                                const math::Vector4Df& data,
                                const Shader*          shader) const {
    return false;
  }

  virtual bool SetUniformbuffer(const Name&            name,
                                const math::Vector2Di& data,
                                const Shader*          shader) const {
    return false;
  }

  virtual bool SetUniformbuffer(const Name&            name,
                                const math::Vector3Di& data,
                                const Shader*          shader) const {
    return false;
  }

  virtual bool SetUniformbuffer(const Name&            name,
                                const math::Vector4Di& data,
                                const Shader*          shader) const {
    return false;
  }

  virtual bool GetUniformbuffer(math::Matrix4d& result,
                                const Name&     name,
                                const Shader*   shader) const {
    return false;
  }

  virtual bool GetUniformbuffer(int&          result,
                                const Name&   name,
                                const Shader* shader) const {
    return false;
  }

  virtual bool GetUniformbuffer(std::uint32_t& result,
                                const Name&    name,
                                const Shader*  shader) const {
    return false;
  }

  virtual bool GetUniformbuffer(float&        result,
                                const Name&   name,
                                const Shader* shader) const {
    return false;
  }

  virtual bool GetUniformbuffer(math::Vector2Df& result,
                                const Name&      name,
                                const Shader*    shader) const {
    return false;
  }

  virtual bool GetUniformbuffer(math::VectorNf<1>& result,
                                const Name&        name,
                                const Shader*      shader) const {
    return false;
  }

  virtual bool GetUniformbuffer(math::Vector4Df& result,
                                const Name&      name,
                                const Shader*    shader) const {
    return false;
  }

  virtual bool GetUniformbuffer(math::Vector2Di& result,
                                const Name&      name,
                                const Shader*    shader) const {
    return false;
  }

  virtual bool GetUniformbuffer(math::Vector3Di& result,
                                const Name&      name,
                                const Shader*    shader) const {
    return false;
  }

  virtual bool GetUniformbuffer(math::Vector4Di& result,
                                const Name&      name,
                                const Shader*    shader) const {
    return false;
  }

  virtual Texture* CreateNullTexture() const { return nullptr; }

  virtual std::shared_ptr<Texture> CreateTextureFromData(
      const ImageData* imageData) const {
    return nullptr;
  }

  virtual Texture* CreateCubeTextureFromData(std::vector<void*> faces,
                                             std::int32_t       width,
                                             std::int32_t       height,
                                             bool               sRGB,
                                             ETextureFormat     textureFormat
                                             = ETextureFormat::RGBA8,
                                             bool createMipmap = false) const {
    return nullptr;
  }

  virtual FrameBuffer* CreateFrameBuffer(const FrameBufferInfo& info) const {
    return nullptr;
  }

  virtual std::shared_ptr<RenderTarget> CreateRenderTarget(
      const RenderTargetInfo& info) const {
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

  // TODO: implement
  // virtual void BeginDebugEvent(CommandBuffer*        commandBuffer,
  //                             const char*            name,
  //                             const math::Vector4Df& color
  //                             = math::ColorGreen) const {}

  virtual void EndDebugEvent(CommandBuffer* commandBuffer) const {}

  virtual void GenerateMips(const Texture* texture) const {}

  virtual void EnableWireframe(bool enable) const {}

  virtual void SetImageTexture(std::int32_t            index,
                               const Texture*          texture,
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

  virtual std::shared_ptr<RenderFrameContext> BeginRenderFrame() {
    return nullptr;
  }

  virtual void EndRenderFrame(
      const std::shared_ptr<RenderFrameContext>& renderFrameContextPtr) {}

  virtual void QueueSubmit(
      const std::shared_ptr<RenderFrameContext>& renderFrameContextPtr,
      class Semaphore*                           signalSemaphore) {}

  virtual RasterizationStateInfo* CreateRasterizationState(
      const RasterizationStateInfo& initializer) const {
    return nullptr;
  }

  virtual StencilOpStateInfo* CreateStencilOpStateInfo(
      const StencilOpStateInfo& initializer) const {
    return nullptr;
  }

  virtual DepthStencilStateInfo* CreateDepthStencilState(
      const DepthStencilStateInfo& initializer) const {
    return nullptr;
  }

  virtual BlendingStateInfo* CreateBlendingState(
      const BlendingStateInfo& initializer) const {
    return nullptr;
  }

  virtual PipelineStateInfo* CreatePipelineStateInfo(
      const PipelineStateFixedInfo*   pipelineStateFixed,
      const GraphicsPipelineShader    shader,
      const VertexBufferArray&        vertexBufferArray,
      const RenderPass*               renderPass,
      const ShaderBindingLayoutArray& shaderBindingArray,
      const PushConstant*             pushConstant,
      std::int32_t                    subpassIndex) const {
    return nullptr;
  }

  virtual PipelineStateInfo* CreateComputePipelineStateInfo(
      const Shader*                   shader,
      const ShaderBindingLayoutArray& shaderBindingArray,
      const PushConstant*             pushConstant) const {
    return nullptr;
  }

  // TODO: implement
  // virtual PipelineStateInfo* CreateRaytracingPipelineStateInfo(
  //    const std::vector<RaytracingPipelineShader>& shaders,
  //    const RaytracingPipelineData&                raytracingData,
  //    const ShaderBindingLayoutArray&              shaderBindingArray,
  //    const PushConstant*                          pushConstant) const {
  //  return nullptr;
  //}

  virtual void RemovePipelineStateInfo(size_t hash) {}

  virtual ShaderBindingLayout* CreateShaderBindings(
      const ShaderBindingArray& shaderBindingArray) const {
    assert(0);
    return nullptr;
  }

  virtual std::shared_ptr<ShaderBindingInstance> CreateShaderBindingInstance(
      const ShaderBindingArray&       shaderBindingArray,
      const ShaderBindingInstanceType type) const {
    assert(0);
    return nullptr;
  }

  // TODO: consider use Dimension or Point instead of Vector2Di
  virtual RenderPass* GetOrCreateRenderPass(
      const std::vector<Attachment>& colorAttachments,
      const math::Vector2Di&         offset,
      const math::Vector2Di&         extent) const {
    return nullptr;
  }

  // TODO: consider use Dimension or Point instead of Vector2Di
  virtual RenderPass* GetOrCreateRenderPass(
      const std::vector<Attachment>& colorAttachments,
      const Attachment&              depthAttachment,
      const math::Vector2Di&         offset,
      const math::Vector2Di&         extent) const {
    return nullptr;
  }

  // TODO: consider use Dimension or Point instead of Vector2Di
  virtual RenderPass* GetOrCreateRenderPass(
      const std::vector<Attachment>& colorAttachments,
      const Attachment&              depthAttachment,
      const Attachment&              colorResolveAttachment,
      const math::Vector2Di&         offset,
      const math::Vector2Di&         extent) const {
    return nullptr;
  }

  // TODO: consider use Dimension or Point instead of Vector2Di
  virtual RenderPass* GetOrCreateRenderPass(
      const RenderPassInfo&  renderPassInfo,
      const math::Vector2Di& offset,
      const math::Vector2Di& extent) const {
    return nullptr;
  }

  virtual CommandBufferManager* GetCommandBufferManager() const {
    return nullptr;
  }

  virtual EMSAASamples GetSelectedMSAASamples() const {
    return EMSAASamples::COUNT_1;
  }

  // ResourceBarrier
  virtual bool TransitionLayout(CommandBuffer*  commandBuffer,
                                Texture*        texture,
                                EResourceLayout newLayout) const {
    return true;
  }

  virtual bool TransitionLayoutImmediate(Texture*        texture,
                                         EResourceLayout newLayout) const {
    return true;
  }

  virtual bool TransitionLayout(CommandBuffer*  commandBuffer,
                                Buffer*         buffer,
                                EResourceLayout newLayout) const {
    return true;
  }

  virtual bool TransitionLayoutImmediate(Buffer*         buffer,
                                         EResourceLayout newLayout) const {
    return true;
  }

  virtual void UAVBarrier(CommandBuffer* commandBuffer,
                          Texture*       texture) const {}

  virtual void UAVBarrierImmediate(Texture* texture) const {}

  virtual void UAVBarrier(CommandBuffer* commandBuffer, Buffer* buffer) const {}

  virtual void UAVBarrierImmediate(Buffer* buffer) const {}

  //////////////////////////////////////////////////////////////////////////

  virtual std::shared_ptr<Swapchain> GetSwapchain() const { return nullptr; }

  virtual class SwapchainImage* GetSwapchainImage(std::int32_t index) const {
    return nullptr;
  }

  virtual void RecreateSwapChain() {}

  virtual std::uint32_t GetMaxSwapchainCount() const { return 0; }

  virtual void BindShadingRateImage(CommandBuffer* commandBuffer,
                                    Texture*       vrstexture) const {}

  virtual MemoryPool* GetMemoryPool() const { return nullptr; }

  virtual void NextSubpass(const CommandBuffer* commandBuffer) const {}

  virtual void BindGraphicsShaderBindingInstances(
      const CommandBuffer*                 commandBuffer,
      const PipelineStateInfo*             piplineState,
      const ShaderBindingInstanceCombiner& shaderBindingInstanceCombiner,
      std::uint32_t                        firstSet) const {}

  virtual void BindComputeShaderBindingInstances(
      const CommandBuffer*                 commandBuffer,
      const PipelineStateInfo*             piplineState,
      const ShaderBindingInstanceCombiner& shaderBindingInstanceCombiner,
      std::uint32_t                        firstSet) const {}

  virtual void BindRaytracingShaderBindingInstances(
      const CommandBuffer*                 commandBuffer,
      const PipelineStateInfo*             piplineState,
      const ShaderBindingInstanceCombiner& shaderBindingInstanceCombiner,
      std::uint32_t                        firstSet) const {}

  virtual FenceManager* GetFenceManager() { return nullptr; }

  virtual SemaphoreManager* GetSemaphoreManager() { return nullptr; }

  virtual std::uint32_t GetCurrentFrameIndex() const { return 0; }

  virtual std::uint32_t GetCurrentFrameNumber() const { return 0; }

  virtual void IncrementFrameNumber() {}

  virtual bool IsSupportVSync() const { return false; }

  virtual bool OnHandleResized(std::uint32_t witdh,
                               std::uint32_t height,
                               bool          isMinimized) {
    return false;
  }

  // virtual RaytracingScene* CreateRaytracingScene() const { return nullptr; }

  virtual CommandBuffer* BeginSingleTimeCommands() const { return nullptr; }

  virtual void EndSingleTimeCommands(CommandBuffer* commandBuffer) const {}

  // RaytracingScene* raytracingScene = nullptr;

  // CreateBuffers
  virtual std::shared_ptr<Buffer> CreateStructuredBuffer(
      std::uint64_t     size,
      std::uint64_t     alignment,
      std::uint64_t     stride,
      EBufferCreateFlag bufferCreateFlag,
      EResourceLayout   initialState,
      const void*       data     = nullptr,
      std::uint64_t     dataSize = 0) const {
    return nullptr;
  }

  virtual std::shared_ptr<Buffer> CreateRawBuffer(
      std::uint64_t     size,
      std::uint64_t     alignment,
      EBufferCreateFlag bufferCreateFlag,
      EResourceLayout   initialState,
      const void*       data     = nullptr,
      std::uint64_t     dataSize = 0) const {
    return nullptr;
  }

  virtual std::shared_ptr<Buffer> CreateFormattedBuffer(
      std::uint64_t     size,
      std::uint64_t     alignment,
      ETextureFormat    format,
      EBufferCreateFlag bufferCreateFlag,
      EResourceLayout   initialState,
      const void*       data     = nullptr,
      std::uint64_t     dataSize = 0) const {
    return nullptr;
  }

  virtual std::shared_ptr<IUniformBufferBlock> CreateUniformBufferBlock(
      Name name, LifeTimeType lifeTimeType, size_t size = 0) const {
    return nullptr;
  }

  virtual std::shared_ptr<VertexBuffer> CreateVertexBuffer(
      const std::shared_ptr<VertexStreamData>& streamData) const {
    return nullptr;
  }

  virtual std::shared_ptr<IndexBuffer> CreateIndexBuffer(
      const std::shared_ptr<IndexStreamData>& streamData) const {
    return nullptr;
  }

  template <typename T = Buffer>
  inline std::shared_ptr<T> CreateStructuredBuffer(
      std::uint64_t     size,
      std::uint64_t     alignment,
      std::uint64_t     stride,
      EBufferCreateFlag bufferCreateFlag,
      EResourceLayout   initialState,
      const void*       data     = nullptr,
      std::uint64_t     dataSize = 0) const {
    return std::static_pointer_cast<T>(CreateStructuredBuffer(size,
                                                              alignment,
                                                              stride,
                                                              bufferCreateFlag,
                                                              initialState,
                                                              data,
                                                              dataSize));
  }

  template <typename T = Buffer>
  inline std::shared_ptr<T> CreateRawBuffer(std::uint64_t     size,
                                            std::uint64_t     alignment,
                                            EBufferCreateFlag bufferCreateFlag,
                                            EResourceLayout   initialState,
                                            const void*       data = nullptr,
                                            std::uint64_t dataSize = 0) const {
    return std::static_pointer_cast<T>(CreateRawBuffer(
        size, alignment, bufferCreateFlag, initialState, data, dataSize));
  }

  template <typename T = Buffer>
  inline std::shared_ptr<T> CreateFormattedBuffer(
      std::uint64_t     size,
      std::uint64_t     alignment,
      ETextureFormat    format,
      EBufferCreateFlag bufferCreateFlag,
      EResourceLayout   initialState,
      const void*       data     = nullptr,
      std::uint64_t     dataSize = 0) const {
    return std::static_pointer_cast<T>(CreateFormattedBuffer(size,
                                                             alignment,
                                                             format,
                                                             bufferCreateFlag,
                                                             initialState,
                                                             data,
                                                             dataSize));
  }

  template <typename T = IUniformBufferBlock>
  inline std::shared_ptr<T> CreateUniformBufferBlock(Name         name,
                                                     LifeTimeType lifeTimeType,
                                                     size_t size = 0) const {
    return std::static_pointer_cast<T>(
        CreateUniformBufferBlock(name, lifeTimeType, size));
  }

  //////////////////////////////////////////////////////////////////////////

  // Create Images
  virtual std::shared_ptr<Texture> Create2DTexture(
      std::uint32_t        witdh,
      std::uint32_t        height,
      std::uint32_t        arrayLayers,
      std::uint32_t        mipLevels,
      ETextureFormat       format,
      ETextureCreateFlag   textureCreateFlag,
      EResourceLayout      imageLayout   = EResourceLayout::UNDEFINED,
      const ImageBulkData& imageBulkData = {},
      const RTClearValue&  clearValue    = RTClearValue::s_kInvalid,
      const wchar_t*       resourceName    = nullptr) const {
    return nullptr;
  }

  virtual std::shared_ptr<Texture> CreateCubeTexture(
      std::uint32_t        witdh,
      std::uint32_t        height,
      std::uint32_t        mipLevels,
      ETextureFormat       format,
      ETextureCreateFlag   textureCreateFlag,
      EResourceLayout      imageLayout   = EResourceLayout::UNDEFINED,
      const ImageBulkData& imageBulkData = {},
      const RTClearValue&  clearValue    = RTClearValue::s_kInvalid,
      const wchar_t*       resourceName    = nullptr) const {
    return nullptr;
  }

  template <typename T>
  std::shared_ptr<T> Create2DTexture(
      std::uint32_t        witdh,
      std::uint32_t        height,
      std::uint32_t        arrayLayers,
      std::uint32_t        mipLevels,
      ETextureFormat       format,
      ETextureCreateFlag   textureCreateFlag,
      EResourceLayout      imageLayout   = EResourceLayout::UNDEFINED,
      const ImageBulkData& imageCopyData = {},
      const RTClearValue&  clearValue    = RTClearValue::s_kInvalid,
      const wchar_t*       resourceName    = nullptr) const {
    return std::static_pointer_cast<T>(Create2DTexture(witdh,
                                                       height,
                                                       arrayLayers,
                                                       mipLevels,
                                                       format,
                                                       textureCreateFlag,
                                                       imageLayout,
                                                       imageCopyData,
                                                       clearValue,
                                                       resourceName));
  }

  template <typename T>
  std::shared_ptr<T> CreateCubeTexture(
      std::uint32_t        witdh,
      std::uint32_t        height,
      std::uint32_t        mipLevels,
      ETextureFormat       format,
      ETextureCreateFlag   textureCreateFlag,
      EResourceLayout      imageLayout   = EResourceLayout::UNDEFINED,
      const ImageBulkData& imageCopyData = {},
      const RTClearValue&  clearValue    = RTClearValue::s_kInvalid,
      const wchar_t*       resourceName    = nullptr) const {
    return std::static_pointer_cast<T>(CreateCubeTexture(witdh,
                                                         height,
                                                         mipLevels,
                                                         format,
                                                         textureCreateFlag,
                                                         imageLayout,
                                                         imageCopyData,
                                                         clearValue,
                                                         resourceName));
  }

  //////////////////////////////////////////////////////////////////////////
};

extern RHI* g_rhi;

// TODO:
// - consider move pipeline state info templated files to another file
// (current problem - g_rhi dependecty so not possible to move in
// pipeline_state_info file)
// - consider naming convention for template parameters

/**
 * \brief Constructs a m_samplerStateInfo object with configurable parameters
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
  static SamplerStateInfo* Create(math::Vector4Df BorderColor
                                  = math::Vector4Df(0.0f, 0.0f, 0.0f, 1.0f)) {
    static SamplerStateInfo* cachedInfo = nullptr;
    if (cachedInfo) {
      return cachedInfo;
    }

    SamplerStateInfo initializer;
    initializer.m_minification_           = TMinification;
    initializer.m_magnification_          = TMagnification;
    initializer.m_addressU_               = TAddressU;
    initializer.m_addressV_               = TAddressV;
    initializer.m_addressW_               = TAddressW;
    initializer.m_mipLODBias_             = TMipLODBias;
    initializer.m_maxAnisotropy_          = TMaxAnisotropy;
    initializer.m_isEnableComparisonMode_ = TIsEnableComparisonMode;
    initializer.m_textureComparisonMode_  = TTextureComparisonMode;
    initializer.m_comparisonFunc_         = TComparisonFunc;
    initializer.m_borderColor_            = BorderColor;
    initializer.m_minLOD_                 = TMinLOD;
    initializer.m_maxLOD_                 = TMaxLOD;
    initializer.GetHash();
    cachedInfo = g_rhi->CreateSamplerState(initializer);
    return cachedInfo;
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
  static RasterizationStateInfo* Create(
      std::optional<EMSAASamples> sampleCountOpt = std::nullopt) {
    static RasterizationStateInfo* cachedInfo = nullptr;
    if (cachedInfo) {
      return cachedInfo;
    }

    RasterizationStateInfo initializer;
    initializer.m_polygonMode_             = TPolygonMode;
    initializer.m_cullMode_                = TCullMode;
    initializer.m_frontFace_               = TFrontFace;
    initializer.m_depthBiasEnable_         = TDepthBiasEnable;
    initializer.m_depthBiasConstantFactor_ = TDepthBiasConstantFactor;
    initializer.m_depthBiasClamp_          = TDepthBiasClamp;
    initializer.m_depthBiasSlopeFactor_    = TDepthBiasSlopeFactor;
    initializer.m_lineWidth_               = TLineWidth;
    initializer.m_depthClampEnable_        = TDepthClampEnable;
    initializer.m_rasterizerDiscardEnable_ = TRasterizerDiscardEnable;

    initializer.m_sampleCount_           = sampleCountOpt.value_or(TSampleCount);
    initializer.m_sampleShadingEnable_   = TSampleShadingEnable;
    initializer.m_minSampleShading_      = TMinSampleShading;
    initializer.m_alphaToCoverageEnable_ = TAlphaToCoverageEnable;
    initializer.m_alphaToOneEnable_      = TAlphaToOneEnable;

    initializer.GetHash();
    // TODO: problem (should be in cpp)
    cachedInfo = g_rhi->CreateRasterizationState(initializer);
    return cachedInfo;
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
  static DepthStencilStateInfo* Create(StencilOpStateInfo* Front = nullptr,
                                       StencilOpStateInfo* Back  = nullptr) {
    static DepthStencilStateInfo* cachedInfo = nullptr;
    if (cachedInfo) {
      return cachedInfo;
    }

    DepthStencilStateInfo initializer;
    initializer.m_depthTestEnable_       = TDepthTestEnable;
    initializer.m_depthWriteEnable_      = TDepthWriteEnable;
    initializer.m_depthCompareOp_        = TDepthCompareOp;
    initializer.m_depthBoundsTestEnable_ = TDepthBoundsTestEnable;
    initializer.m_stencilTestEnable_     = TStencilTestEnable;
    initializer.m_front_                 = Front;
    initializer.m_back_                  = Back;
    initializer.m_minDepthBounds_        = TMinDepthBounds;
    initializer.m_maxDepthBounds_        = TMaxDepthBounds;
    initializer.GetHash();
    cachedInfo = g_rhi->CreateDepthStencilState(initializer);
    return cachedInfo;
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
  static BlendingStateInfo* Create() {
    static BlendingStateInfo* cachedInfo = nullptr;
    if (cachedInfo) {
      return cachedInfo;
    }

    BlendingStateInfo initializer;
    initializer.m_blendEnable_    = TBlendEnable;
    initializer.m_src_            = TSrc;
    initializer.m_dest_           = TDest;
    initializer.m_blendOp_        = TBlendOp;
    initializer.m_srcAlpha_       = TSrcAlpha;
    initializer.m_destAlpha_      = TDestAlpha;
    initializer.m_alphaBlendOp_   = TAlphaBlendOp;
    initializer.m_colorWriteMask_ = TColorWriteMask;
    initializer.GetHash();
    cachedInfo = g_rhi->CreateBlendingState(initializer);
    return cachedInfo;
  }
};

}  // namespace game_engine

#endif  // GAME_ENGINE_RHI_H