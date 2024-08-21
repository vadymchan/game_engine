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
  T* createShader(const ShaderInfo& shaderInfo) const {
    return (T*)s_shaderPool.getOrCreate<ShaderInfo, T>(shaderInfo);
  }

  void addShader(const ShaderInfo& shaderInfo, Shader* shader) {
    return s_shaderPool.add(shaderInfo, shader);
  }

  void releaseShader(const ShaderInfo& shaderInfo) {
    s_shaderPool.release(shaderInfo);
  }

  std::vector<Shader*> getAllShaders() {
    std::vector<Shader*> Out;
    s_shaderPool.getAllResource(Out);
    return Out;
  }

  // END: shader related functions and variables
  // =================================================================

  virtual Name getRHIName() { return Name::s_kInvalid; }

  virtual bool init(const std::shared_ptr<Window>& window);
  virtual void onInitRHI();
  virtual void release();

  virtual void* getWindow() const { return nullptr; }

  virtual SamplerStateInfo* createSamplerState(
      const SamplerStateInfo& info) const {
    return nullptr;
  }

  // TODO: not used / overriden (consider remove)
  virtual void releaseSamplerState(SamplerStateInfo* samplerState) const {}

  // TODO: not used / overriden (consider remove)
  virtual void bindSamplerState(std::int32_t            index,
                                const SamplerStateInfo* samplerState) const {}

  // TODO: not used / overriden (consider remove)
  virtual void setClear(ERenderBufferType typeBit) const {}

  // TODO: not used / overriden (consider remove)
  virtual void setClearColor(float r, float g, float b, float a) const {}

  // TODO: not used / overriden (consider remove)
  virtual void setClearColor(math::Vector4Df rgba) const {}

  // TODO: not used / overriden (consider remove)
  virtual void setClearBuffer(ERenderBufferType typeBit,
                              const float*      value,
                              std::int32_t      bufferIndex) const {}

  // TODO: not used / overriden (consider remove)
  virtual void setClearBuffer(ERenderBufferType   typeBit,
                              const std::int32_t* value,
                              std::int32_t        bufferIndex) const {}

  // TODO: not used / overriden (consider remove)
  virtual void setFrameBuffer(const FrameBuffer* rt,
                              std::int32_t       index = 0,
                              bool               mrt   = false) const {}

  // TODO: not used / overriden (consider remove)
  virtual void setDrawBuffers(
      const std::initializer_list<EDrawBufferType>& list) const {}

  virtual void setTextureFilter(ETextureType         type,
                                std::int32_t         sampleCount,
                                ETextureFilterTarget target,
                                ETextureFilter       filter) const {}

  // TODO: not used / overriden (consider remove)
  virtual void setTextureWrap(int flag) const {}

  // TODO: not used / overriden (consider remove)
  virtual void setTexture(std::int32_t index, const Texture* texture) const {}

  virtual void drawArrays(
      const std::shared_ptr<RenderFrameContext>& renderFrameContext,
      // EPrimitiveType                          type,
      std::int32_t                               vertStartIndex,
      std::int32_t                               vertCount) const {}

  virtual void drawArraysInstanced(
      const std::shared_ptr<RenderFrameContext>& renderFrameContext,
      // EPrimitiveType                          type,
      std::int32_t                               vertStartIndex,
      std::int32_t                               vertCount,
      std::int32_t                               instanceCount) const {}

  virtual void drawElements(
      const std::shared_ptr<RenderFrameContext>& renderFrameContext,
      // EPrimitiveType                          type,
      std::int32_t                               elementSize,
      std::int32_t                               startIndex,
      std::int32_t                               indexCount) const {}

  virtual void drawElementsInstanced(
      const std::shared_ptr<RenderFrameContext>& renderFrameContext,
      // EPrimitiveType                          type,
      std::int32_t                               elementSize,
      std::int32_t                               startIndex,
      std::int32_t                               indexCount,
      std::int32_t                               instanceCount) const {}

  virtual void drawElementsBaseVertex(
      const std::shared_ptr<RenderFrameContext>& renderFrameContext,
      // EPrimitiveType                          type,
      std::int32_t                               elementSize,
      std::int32_t                               startIndex,
      std::int32_t                               indexCount,
      std::int32_t                               baseVertexIndex) const {}

  virtual void drawElementsInstancedBaseVertex(
      const std::shared_ptr<RenderFrameContext>& renderFrameContext,
      // EPrimitiveType                          type,
      std::int32_t                               elementSize,
      std::int32_t                               startIndex,
      std::int32_t                               indexCount,
      std::int32_t                               baseVertexIndex,
      std::int32_t                               instanceCount) const {}

  virtual void drawIndirect(
      const std::shared_ptr<RenderFrameContext>& renderFrameContext,
      // EPrimitiveType                          type,
      Buffer*                                    buffer,
      std::int32_t                               startIndex,
      std::int32_t                               drawCount) const {}

  virtual void drawElementsIndirect(
      const std::shared_ptr<RenderFrameContext>& renderFrameContext,
      // EPrimitiveType                          type,
      Buffer*                                    buffer,
      std::int32_t                               startIndex,
      std::int32_t                               drawCount) const {}

  virtual void dispatchCompute(
      const std::shared_ptr<RenderFrameContext>& renderFrameContext,
      std::uint32_t                              numGroupsX,
      std::uint32_t                              numGroupsY,
      std::uint32_t                              numGroupsZ) const {}

  // TODO: implement in future iterations
  // virtual void dispatchRay(
  //    const std::shared_ptr<RenderFrameContext>& renderFrameContext,
  //    const RaytracingDispatchData&              dispatchData) const {}

  // TODO: not used / overriden (consider remove)
  virtual void enableDepthBias(bool         enable,
                               EPolygonMode polygonMode
                               = EPolygonMode::FILL) const {}

  // TODO: not used / overriden (consider remove)
  virtual void setDepthBias(float constant, float slope) const {}

  virtual bool createShaderInternal(Shader*           shader,
                                    const ShaderInfo& shaderInfo) const {
    return false;
  }

  virtual void releaseShader(Shader* shader) const {}

  // TODO: not used / overriden (consider remove)
  virtual void setViewport(std::int32_t x,
                           std::int32_t y,
                           std::int32_t width,
                           std::int32_t height) const {}

  // TODO: not used / overriden (consider remove)
  virtual void setViewport(const Viewport& viewport) const {}

  // TODO: not used / overriden (consider remove)
  virtual void setViewportIndexed(
      std::int32_t index, float x, float y, float width, float height) const {}

  // TODO: not used / overriden (consider remove)
  virtual void setViewportIndexed(std::int32_t    index,
                                  const Viewport& viewport) const {}

  // TODO: not used / overriden (consider remove)
  virtual void setViewportIndexedArray(std::int32_t    startIndex,
                                       std::int32_t    count,
                                       const Viewport* viewports) const {}

  // TODO: not used / overriden (consider remove)
  virtual bool setUniformbuffer(const Name&           name,
                                const math::Matrix4d& data,
                                const Shader*         shader) const {
    return false;
  }

  // TODO: seems not used (it's actually used but in dead code)and overriden
  // (consider remove)
  virtual bool setUniformbuffer(const Name&   name,
                                const int     data,
                                const Shader* shader) const {
    return false;
  }

  // TODO: not used / overriden (consider remove)
  virtual bool setUniformbuffer(const Name&         name,
                                const std::uint32_t data,
                                const Shader*       shader) const {
    return false;
  }

  // TODO: not used / overriden (consider remove)
  inline virtual bool setUniformbuffer(Name          name,
                                       const bool    data,
                                       const Shader* shader) const {
    return setUniformbuffer(name, (std::int32_t)data, shader);
  }

  // TODO: not used / overriden (consider remove)
  virtual bool setUniformbuffer(const Name&   name,
                                const float   data,
                                const Shader* shader) const {
    return false;
  }

  // TODO: not used / overriden (consider remove)
  virtual bool setUniformbuffer(const Name&            name,
                                const math::Vector2Df& data,
                                const Shader*          shader) const {
    return false;
  }

  // TODO: not used / overriden (consider remove)
  virtual bool setUniformbuffer(const Name&              name,
                                const math::VectorNf<1>& data,
                                const Shader*            shader) const {
    return false;
  }

  // TODO: not used / overriden (consider remove)
  virtual bool setUniformbuffer(const Name&            name,
                                const math::Vector4Df& data,
                                const Shader*          shader) const {
    return false;
  }

  // TODO: not used / overriden (consider remove)
  virtual bool setUniformbuffer(const Name&            name,
                                const math::Vector2Di& data,
                                const Shader*          shader) const {
    return false;
  }

  // TODO: not used / overriden (consider remove)
  virtual bool setUniformbuffer(const Name&            name,
                                const math::Vector3Di& data,
                                const Shader*          shader) const {
    return false;
  }

  // TODO: not used / overriden (consider remove)
  virtual bool setUniformbuffer(const Name&            name,
                                const math::Vector4Di& data,
                                const Shader*          shader) const {
    return false;
  }

  // TODO: not used / overriden (consider remove)
  virtual bool getUniformbuffer(math::Matrix4d& result,
                                const Name&     name,
                                const Shader*   shader) const {
    return false;
  }

  // TODO: not used / overriden (consider remove)
  virtual bool getUniformbuffer(int&          result,
                                const Name&   name,
                                const Shader* shader) const {
    return false;
  }

  // TODO: not used / overriden (consider remove)
  virtual bool getUniformbuffer(std::uint32_t& result,
                                const Name&    name,
                                const Shader*  shader) const {
    return false;
  }

  // TODO: not used / overriden (consider remove)
  virtual bool getUniformbuffer(float&        result,
                                const Name&   name,
                                const Shader* shader) const {
    return false;
  }

  // TODO: not used / overriden (consider remove)
  virtual bool getUniformbuffer(math::Vector2Df& result,
                                const Name&      name,
                                const Shader*    shader) const {
    return false;
  }

  // TODO: not used / overriden (consider remove)
  virtual bool getUniformbuffer(math::VectorNf<1>& result,
                                const Name&        name,
                                const Shader*      shader) const {
    return false;
  }

  // TODO: not used / overriden (consider remove)
  virtual bool getUniformbuffer(math::Vector4Df& result,
                                const Name&      name,
                                const Shader*    shader) const {
    return false;
  }

  // TODO: not used / overriden (consider remove)
  virtual bool getUniformbuffer(math::Vector2Di& result,
                                const Name&      name,
                                const Shader*    shader) const {
    return false;
  }

  // TODO: not used / overriden (consider remove)
  virtual bool getUniformbuffer(math::Vector3Di& result,
                                const Name&      name,
                                const Shader*    shader) const {
    return false;
  }

  // TODO: not used / overriden (consider remove)
  virtual bool getUniformbuffer(math::Vector4Di& result,
                                const Name&      name,
                                const Shader*    shader) const {
    return false;
  }

  // TODO: not used / overriden (consider remove)
  virtual Texture* createNullTexture() const { return nullptr; }

  virtual std::shared_ptr<Texture> createTextureFromData(
      const ImageData* imageData) const {
    return nullptr;
  }

  // TODO: not used / overriden (consider remove)
  virtual Texture* createCubeTextureFromData(std::vector<void*> faces,
                                             std::int32_t       width,
                                             std::int32_t       height,
                                             bool               sRGB,
                                             ETextureFormat     textureFormat
                                             = ETextureFormat::RGBA8,
                                             bool createMipmap = false) const {
    return nullptr;
  }

  // TODO: not used / overriden (consider remove)
  virtual FrameBuffer* createFrameBuffer(const FrameBufferInfo& info) const {
    return nullptr;
  }

  // TODO: not used / overriden (consider remove)
  virtual std::shared_ptr<RenderTarget> createRenderTarget(
      const RenderTargetInfo& info) const {
    return nullptr;
  }

  // TODO: the following methods related to render setting configurations and they may be POC in future

  // TODO: not used / overriden 
  virtual void enableDepthTest(bool enable) const {}

  // TODO: not used / overriden 
  virtual void enableBlend(bool enable) const {}

  // TODO: not used / overriden 
  virtual void setBlendFunc(EBlendFactor src, EBlendFactor dest) const {}

  // TODO: not used / overriden 
  virtual void setBlendFuncRT(EBlendFactor src,
                              EBlendFactor dest,
                              std::int32_t rtIndex = 0) const {}

  // TODO: not used / overriden 
  virtual void setBlendEquation(EBlendOp func) const {}

  // TODO: not used / overriden 
  virtual void setBlendEquation(EBlendOp func, std::int32_t rtIndex) const {}

  // TODO: not used / overriden 
  virtual void setBlendColor(float r, float g, float b, float a) const {}

  // TODO: not used / overriden 
  virtual void enableStencil(bool enable) const {}

  // TODO: not used / overriden 
  virtual void setStencilOpSeparate(EFace      face,
                                    EStencilOp sFail,
                                    EStencilOp dpFail,
                                    EStencilOp dpPass) const {}

  // TODO: not used / overriden 
  virtual void setStencilFunc(ECompareOp    func,
                              std::int32_t  ref,
                              std::uint32_t mask) const {}

  // TODO: not used / overriden 
  virtual void setDepthFunc(ECompareOp func) const {}

  // TODO: not used / overriden 
  virtual void setDepthMask(bool enable) const {}

  // TODO: not used / overriden 
  virtual void setColorMask(bool r, bool g, bool b, bool a) const {}

  // TODO: not used / overriden 
  virtual void enableSRGB(bool enable) const {}

  // TODO: not used / overriden 
  virtual void enableDepthClip(bool enable) const {}

  // TODO: either implement in future or remove
  virtual void beginDebugEvent(const char* name) const {}

  // TODO: either implement in future or remove
  virtual void endDebugEvent() const {}

  // TODO: either implement in future or remove
  // virtual void beginDebugEvent(CommandBuffer*        commandBuffer,
  //                             const char*            name,
  //                             const math::Vector4Df& color
  //                             = math::g_colorGreen) const {}

  // TODO: either implement in future or remove
  virtual void endDebugEvent(CommandBuffer* commandBuffer) const {}

  // TODO: not used / overriden 
  virtual void generateMips(const Texture* texture) const {}

  // TODO: not used / overriden 
  virtual void enableWireframe(bool enable) const {}

  // TODO: not used / overriden 
  virtual void setImageTexture(std::int32_t            index,
                               const Texture*          texture,
                               EImageTextureAccessType type) const {}

  // TODO: not used / overriden 
  virtual void setPolygonMode(EFace        face,
                              EPolygonMode mode = EPolygonMode::FILL) {}

  // TODO: not used / overriden 
  virtual void enableRasterizerDiscard(bool enable) const {}

  // TODO: not used / overriden 
  virtual void setTextureMipmapLevelLimit(ETextureType type,
                                          std::int32_t sampleCount,
                                          std::int32_t baseLevel,
                                          std::int32_t maxLevel) const {}

  // TODO: not used / overriden 
  virtual void enableMultisample(bool enable) const {}

  // TODO: not used / overriden 
  virtual void setCubeMapSeamless(bool enable) const {}

  // TODO: not used / overriden 
  virtual void setLineWidth(float width) const {}

  virtual void flush() const {}

  virtual void finish() const {}

  virtual std::shared_ptr<RenderFrameContext> beginRenderFrame() {
    return nullptr;
  }

  virtual void endRenderFrame(
      const std::shared_ptr<RenderFrameContext>& renderFrameContextPtr) {}

  virtual void queueSubmit(
      const std::shared_ptr<RenderFrameContext>& renderFrameContextPtr,
      class Semaphore*                           signalSemaphore) {}

  virtual RasterizationStateInfo* createRasterizationState(
      const RasterizationStateInfo& initializer) const {
    return nullptr;
  }

  virtual StencilOpStateInfo* createStencilOpStateInfo(
      const StencilOpStateInfo& initializer) const {
    return nullptr;
  }

  virtual DepthStencilStateInfo* createDepthStencilState(
      const DepthStencilStateInfo& initializer) const {
    return nullptr;
  }

  virtual BlendingStateInfo* createBlendingState(
      const BlendingStateInfo& initializer) const {
    return nullptr;
  }

  virtual PipelineStateInfo* createPipelineStateInfo(
      const PipelineStateFixedInfo*   pipelineStateFixed,
      const GraphicsPipelineShader    shader,
      const VertexBufferArray&        vertexBufferArray,
      const RenderPass*               renderPass,
      const ShaderBindingLayoutArray& shaderBindingArray,
      const PushConstant*             pushConstant,
      std::int32_t                    subpassIndex) const {
    return nullptr;
  }

  virtual PipelineStateInfo* createComputePipelineStateInfo(
      const Shader*                   shader,
      const ShaderBindingLayoutArray& shaderBindingArray,
      const PushConstant*             pushConstant) const {
    return nullptr;
  }

  // TODO: implement
  // virtual PipelineStateInfo* createRaytracingPipelineStateInfo(
  //    const std::vector<RaytracingPipelineShader>& shaders,
  //    const RaytracingPipelineData&                raytracingData,
  //    const ShaderBindingLayoutArray&              shaderBindingArray,
  //    const PushConstant*                          pushConstant) const {
  //  return nullptr;
  //}

  virtual void removePipelineStateInfo(size_t hash) {}

  virtual ShaderBindingLayout* createShaderBindings(
      const ShaderBindingArray& shaderBindingArray) const {
    assert(0);
    return nullptr;
  }

  virtual std::shared_ptr<ShaderBindingInstance> createShaderBindingInstance(
      const ShaderBindingArray&       shaderBindingArray,
      const ShaderBindingInstanceType type) const {
    assert(0);
    return nullptr;
  }

  // TODO: consider use Dimension or Point instead of Vector2Di
  virtual RenderPass* getOrCreateRenderPass(
      const std::vector<Attachment>& colorAttachments,
      const math::Vector2Di&         offset,
      const math::Vector2Di&         extent) const {
    return nullptr;
  }

  // TODO: consider use Dimension or Point instead of Vector2Di
  virtual RenderPass* getOrCreateRenderPass(
      const std::vector<Attachment>& colorAttachments,
      const Attachment&              depthAttachment,
      const math::Vector2Di&         offset,
      const math::Vector2Di&         extent) const {
    return nullptr;
  }

  // TODO: consider use Dimension or Point instead of Vector2Di
  virtual RenderPass* getOrCreateRenderPass(
      const std::vector<Attachment>& colorAttachments,
      const Attachment&              depthAttachment,
      const Attachment&              colorResolveAttachment,
      const math::Vector2Di&         offset,
      const math::Vector2Di&         extent) const {
    return nullptr;
  }

  // TODO: consider use Dimension or Point instead of Vector2Di
  virtual RenderPass* getOrCreateRenderPass(
      const RenderPassInfo&  renderPassInfo,
      const math::Vector2Di& offset,
      const math::Vector2Di& extent) const {
    return nullptr;
  }

  virtual CommandBufferManager* getCommandBufferManager() const {
    return nullptr;
  }

  virtual EMSAASamples getSelectedMSAASamples() const {
    return EMSAASamples::COUNT_1;
  }

  // ResourceBarrier
  virtual bool transitionLayout(CommandBuffer*  commandBuffer,
                                Texture*        texture,
                                EResourceLayout newLayout) const {
    return true;
  }

  virtual bool transitionLayoutImmediate(Texture*        texture,
                                         EResourceLayout newLayout) const {
    return true;
  }

  virtual bool transitionLayout(CommandBuffer*  commandBuffer,
                                Buffer*         buffer,
                                EResourceLayout newLayout) const {
    return true;
  }

  virtual bool transitionLayoutImmediate(Buffer*         buffer,
                                         EResourceLayout newLayout) const {
    return true;
  }

  virtual void uavBarrier(CommandBuffer* commandBuffer,
                          Texture*       texture) const {}

  virtual void uavBarrierImmediate(Texture* texture) const {}

  virtual void uavBarrier(CommandBuffer* commandBuffer, Buffer* buffer) const {}

  virtual void uavBarrierImmediate(Buffer* buffer) const {}

  //////////////////////////////////////////////////////////////////////////

  virtual std::shared_ptr<Swapchain> getSwapchain() const { return nullptr; }

  virtual class SwapchainImage* getSwapchainImage(std::int32_t index) const {
    return nullptr;
  }

  virtual void recreateSwapChain() {}

  // TODO: not used / overriden 
  virtual std::uint32_t getMaxSwapchainCount() const { return 0; }

  virtual void bindShadingRateImage(CommandBuffer* commandBuffer,
                                    Texture*       vrstexture) const {}

  virtual MemoryPool* getMemoryPool() const { return nullptr; }

  virtual void nextSubpass(const CommandBuffer* commandBuffer) const {}

  virtual void bindGraphicsShaderBindingInstances(
      const CommandBuffer*                 commandBuffer,
      const PipelineStateInfo*             piplineState,
      const ShaderBindingInstanceCombiner& shaderBindingInstanceCombiner,
      std::uint32_t                        firstSet) const {}

  virtual void bindComputeShaderBindingInstances(
      const CommandBuffer*                 commandBuffer,
      const PipelineStateInfo*             piplineState,
      const ShaderBindingInstanceCombiner& shaderBindingInstanceCombiner,
      std::uint32_t                        firstSet) const {}

  virtual void bindRaytracingShaderBindingInstances(
      const CommandBuffer*                 commandBuffer,
      const PipelineStateInfo*             piplineState,
      const ShaderBindingInstanceCombiner& shaderBindingInstanceCombiner,
      std::uint32_t                        firstSet) const {}

  virtual FenceManager* getFenceManager() { return nullptr; }

  virtual SemaphoreManager* getSemaphoreManager() { return nullptr; }

  virtual std::uint32_t getCurrentFrameIndex() const { return 0; }

  virtual std::uint32_t getCurrentFrameNumber() const { return 0; }

  virtual void incrementFrameNumber() {}

  // TODO: not implemented
  virtual bool isSupportVSync() const { return false; }

  virtual bool onHandleResized(std::uint32_t witdh,
                               std::uint32_t height,
                               bool          isMinimized) {
    return false;
  }

  // virtual RaytracingScene* createRaytracingScene() const { return nullptr; }

  virtual CommandBuffer* beginSingleTimeCommands() const { return nullptr; }

  virtual void endSingleTimeCommands(CommandBuffer* commandBuffer) const {}

  // RaytracingScene* raytracingScene = nullptr;

  // CreateBuffers
  virtual std::shared_ptr<Buffer> createStructuredBuffer(
      std::uint64_t     size,
      std::uint64_t     alignment,
      std::uint64_t     stride,
      EBufferCreateFlag bufferCreateFlag,
      EResourceLayout   initialState,
      const void*       data     = nullptr,
      std::uint64_t     dataSize = 0) const {
    return nullptr;
  }

  virtual std::shared_ptr<Buffer> createRawBuffer(
      std::uint64_t     size,
      std::uint64_t     alignment,
      EBufferCreateFlag bufferCreateFlag,
      EResourceLayout   initialState,
      const void*       data     = nullptr,
      std::uint64_t     dataSize = 0) const {
    return nullptr;
  }

  virtual std::shared_ptr<Buffer> createFormattedBuffer(
      std::uint64_t     size,
      std::uint64_t     alignment,
      ETextureFormat    format,
      EBufferCreateFlag bufferCreateFlag,
      EResourceLayout   initialState,
      const void*       data     = nullptr,
      std::uint64_t     dataSize = 0) const {
    return nullptr;
  }

  virtual std::shared_ptr<IUniformBufferBlock> createUniformBufferBlock(
      Name name, LifeTimeType lifeTimeType, size_t size = 0) const {
    return nullptr;
  }

  virtual std::shared_ptr<VertexBuffer> createVertexBuffer(
      const std::shared_ptr<VertexStreamData>& streamData) const {
    return nullptr;
  }

  virtual std::shared_ptr<IndexBuffer> createIndexBuffer(
      const std::shared_ptr<IndexStreamData>& streamData) const {
    return nullptr;
  }

  template <typename T = Buffer>
  inline std::shared_ptr<T> createStructuredBuffer(
      std::uint64_t     size,
      std::uint64_t     alignment,
      std::uint64_t     stride,
      EBufferCreateFlag bufferCreateFlag,
      EResourceLayout   initialState,
      const void*       data     = nullptr,
      std::uint64_t     dataSize = 0) const {
    return std::static_pointer_cast<T>(createStructuredBuffer(size,
                                                              alignment,
                                                              stride,
                                                              bufferCreateFlag,
                                                              initialState,
                                                              data,
                                                              dataSize));
  }

  template <typename T = Buffer>
  inline std::shared_ptr<T> createRawBuffer(std::uint64_t     size,
                                            std::uint64_t     alignment,
                                            EBufferCreateFlag bufferCreateFlag,
                                            EResourceLayout   initialState,
                                            const void*       data = nullptr,
                                            std::uint64_t dataSize = 0) const {
    return std::static_pointer_cast<T>(createRawBuffer(
        size, alignment, bufferCreateFlag, initialState, data, dataSize));
  }

  template <typename T = Buffer>
  inline std::shared_ptr<T> createFormattedBuffer(
      std::uint64_t     size,
      std::uint64_t     alignment,
      ETextureFormat    format,
      EBufferCreateFlag bufferCreateFlag,
      EResourceLayout   initialState,
      const void*       data     = nullptr,
      std::uint64_t     dataSize = 0) const {
    return std::static_pointer_cast<T>(createFormattedBuffer(size,
                                                             alignment,
                                                             format,
                                                             bufferCreateFlag,
                                                             initialState,
                                                             data,
                                                             dataSize));
  }

  template <typename T = IUniformBufferBlock>
  inline std::shared_ptr<T> createUniformBufferBlock(Name         name,
                                                     LifeTimeType lifeTimeType,
                                                     size_t size = 0) const {
    return std::static_pointer_cast<T>(
        createUniformBufferBlock(name, lifeTimeType, size));
  }

  //////////////////////////////////////////////////////////////////////////

  // Create Images
  virtual std::shared_ptr<Texture> create2DTexture(
      std::uint32_t        witdh,
      std::uint32_t        height,
      std::uint32_t        arrayLayers,
      std::uint32_t        mipLevels,
      ETextureFormat       format,
      ETextureCreateFlag   textureCreateFlag,
      EResourceLayout      imageLayout   = EResourceLayout::UNDEFINED,
      const ImageBulkData& imageBulkData = {},
      const RTClearValue&  clearValue    = RTClearValue::s_kInvalid,
      const wchar_t*       resourceName  = nullptr) const {
    return nullptr;
  }

  virtual std::shared_ptr<Texture> createCubeTexture(
      std::uint32_t        witdh,
      std::uint32_t        height,
      std::uint32_t        mipLevels,
      ETextureFormat       format,
      ETextureCreateFlag   textureCreateFlag,
      EResourceLayout      imageLayout   = EResourceLayout::UNDEFINED,
      const ImageBulkData& imageBulkData = {},
      const RTClearValue&  clearValue    = RTClearValue::s_kInvalid,
      const wchar_t*       resourceName  = nullptr) const {
    return nullptr;
  }

  template <typename T>
  std::shared_ptr<T> create2DTexture(
      std::uint32_t        witdh,
      std::uint32_t        height,
      std::uint32_t        arrayLayers,
      std::uint32_t        mipLevels,
      ETextureFormat       format,
      ETextureCreateFlag   textureCreateFlag,
      EResourceLayout      imageLayout   = EResourceLayout::UNDEFINED,
      const ImageBulkData& imageCopyData = {},
      const RTClearValue&  clearValue    = RTClearValue::s_kInvalid,
      const wchar_t*       resourceName  = nullptr) const {
    return std::static_pointer_cast<T>(create2DTexture(witdh,
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
  std::shared_ptr<T> createCubeTexture(
      std::uint32_t        witdh,
      std::uint32_t        height,
      std::uint32_t        mipLevels,
      ETextureFormat       format,
      ETextureCreateFlag   textureCreateFlag,
      EResourceLayout      imageLayout   = EResourceLayout::UNDEFINED,
      const ImageBulkData& imageCopyData = {},
      const RTClearValue&  clearValue    = RTClearValue::s_kInvalid,
      const wchar_t*       resourceName  = nullptr) const {
    return std::static_pointer_cast<T>(createCubeTexture(witdh,
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
  static SamplerStateInfo* s_create(math::Vector4Df BorderColor
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
    initializer.getHash();
    cachedInfo = g_rhi->createSamplerState(initializer);
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
  static RasterizationStateInfo* s_create(
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

    initializer.m_sampleCount_         = sampleCountOpt.value_or(TSampleCount);
    initializer.m_sampleShadingEnable_ = TSampleShadingEnable;
    initializer.m_minSampleShading_    = TMinSampleShading;
    initializer.m_alphaToCoverageEnable_ = TAlphaToCoverageEnable;
    initializer.m_alphaToOneEnable_      = TAlphaToOneEnable;

    initializer.getHash();
    // TODO: problem (should be in cpp)
    cachedInfo = g_rhi->createRasterizationState(initializer);
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
  static DepthStencilStateInfo* s_create(StencilOpStateInfo* Front = nullptr,
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
    initializer.getHash();
    cachedInfo = g_rhi->createDepthStencilState(initializer);
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
  static BlendingStateInfo* s_create() {
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
    initializer.getHash();
    cachedInfo = g_rhi->createBlendingState(initializer);
    return cachedInfo;
  }
};

}  // namespace game_engine

#endif  // GAME_ENGINE_RHI_H