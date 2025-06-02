#ifndef GAME_ENGINE_RENDERER_H
#define GAME_ENGINE_RENDERER_H

#include "gfx/renderer/frame_resources.h"
#include "gfx/renderer/passes/base_pass.h"
#include "gfx/renderer/passes/debug_pass.h"
#include "gfx/renderer/passes/final_pass.h"
#include "gfx/renderer/render_resource_manager.h"
#include "gfx/rhi/common/rhi_enums.h"
#include "gfx/rhi/interface/command_buffer.h"
#include "gfx/rhi/interface/device.h"
#include "gfx/rhi/interface/swap_chain.h"
#include "gfx/rhi/interface/synchronization.h"
#include "gfx/rhi/shader_manager.h"

#include <memory>

namespace game_engine {
class Scene;
}  // namespace game_engine

namespace game_engine {
namespace gfx {
namespace renderer {

/**
 * Coordinates the rendering process, manages frame synchronization
 */
class Renderer {
  public:
  Renderer() = default;

  ~Renderer() { waitForAllFrames_(); }

  bool initialize(Window* window, rhi::RenderingApi api);

  RenderContext beginFrame(Scene* scene, const RenderSettings& renderSettings);
  void          renderFrame(RenderContext& context);
  void          endFrame(RenderContext& context);

  bool onWindowResize(uint32_t width, uint32_t height);
  bool onViewportResize(const math::Dimension2Di& newDimension);

  rhi::Device*           getDevice() const { return m_device.get(); }
  uint32_t               getFrameIndex() const { return m_frameIndex; }
  rhi::ShaderManager*    getShaderManager() const { return m_shaderManager.get(); }
  FrameResources*        getFrameResources() const { return m_frameResources.get(); }
  RenderResourceManager* getResourceManager() const { return m_resourceManager.get(); }

  private:
  void initializeGpuProfiler_();

  std::unique_ptr<rhi::CommandBuffer> acquireCommandBuffer_();

  void recycleCommandBuffer_(std::unique_ptr<rhi::CommandBuffer> cmdBuffer);

  void waitForAllFrames_();

  void setupRenderPasses_();

  static constexpr uint32_t MAX_FRAMES_IN_FLIGHT      = 2;
  static constexpr uint32_t COMMAND_BUFFERS_PER_FRAME = 8;
  static constexpr uint32_t INITIAL_COMMAND_BUFFERS   = 2;

  Window*                                m_window = nullptr;
  std::unique_ptr<rhi::Device>           m_device;
  std::unique_ptr<rhi::SwapChain>        m_swapChain;
  std::unique_ptr<rhi::ShaderManager>    m_shaderManager;
  std::unique_ptr<RenderResourceManager> m_resourceManager;
  std::unique_ptr<FrameResources>        m_frameResources;

  std::unique_ptr<BasePass>  m_basePass;
  std::unique_ptr<FinalPass> m_finalPass;
  std::unique_ptr<DebugPass> m_debugPass;

  // one pool per frame in flight
  std::array<std::vector<std::unique_ptr<rhi::CommandBuffer>>, MAX_FRAMES_IN_FLIGHT> m_commandBufferPools;

  std::array<std::unique_ptr<rhi::Fence>, MAX_FRAMES_IN_FLIGHT>     m_frameFences;
  std::array<std::unique_ptr<rhi::Semaphore>, MAX_FRAMES_IN_FLIGHT> m_imageAvailableSemaphores;
  std::array<std::unique_ptr<rhi::Semaphore>, MAX_FRAMES_IN_FLIGHT> m_renderFinishedSemaphores;

  uint32_t m_currentFrame = 0;
  uint32_t m_frameIndex   = 0;
  bool     m_initialized  = false;
};

}  // namespace renderer
}  // namespace gfx
}  // namespace game_engine

#endif  // GAME_ENGINE_RENDERER_H