#include "gfx/renderer/renderer.h"

#include "ecs/components/camera.h"
#include "gfx/rhi/backends/dx12/command_buffer_dx12.h"
#include "gfx/rhi/common/rhi_creators.h"
#include "platform/common/window.h"
#include "scene/scene_manager.h"
#include "utils/resource/resource_deletion_manager.h"

namespace game_engine {
namespace gfx {
namespace renderer {
bool Renderer::initialize(Window* window, rhi::RenderingApi api) {
  m_window = window;

  rhi::DeviceDesc deviceDesc;
  deviceDesc.window = window;
  m_device          = g_createDevice(api, deviceDesc);

  if (!m_device) {
    GlobalLogger::Log(LogLevel::Error, "Failed to create device");
    return false;
  }

  // TODO: move to a separate function
  rhi::SwapchainDesc swapchainDesc;
  swapchainDesc.width       = window->getSize().width();
  swapchainDesc.height      = window->getSize().height();
  swapchainDesc.format      = rhi::TextureFormat::Bgra8;
  swapchainDesc.bufferCount = MAX_FRAMES_IN_FLIGHT;

  m_swapChain = m_device->createSwapChain(swapchainDesc);
  if (!m_swapChain) {
    GlobalLogger::Log(LogLevel::Error, "Failed to create swap chain");
    return false;
  }

  m_shaderManager   = std::make_unique<rhi::ShaderManager>(m_device.get(), MAX_FRAMES_IN_FLIGHT, true);
  m_resourceManager = std::make_unique<RenderResourceManager>();

  m_frameResources = std::make_unique<FrameResources>(m_device.get(), getResourceManager());
  m_frameResources->initialize(MAX_FRAMES_IN_FLIGHT);
  m_frameResources->resize(window->getSize());

  // synchronization (move to a separate function)
  for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
    rhi::FenceDesc fenceDesc;
    fenceDesc.signaled = true;
    m_frameFences[i]   = m_device->createFence(fenceDesc);

    m_imageAvailableSemaphores[i] = m_device->createSemaphore();
    m_renderFinishedSemaphores[i] = m_device->createSemaphore();

    m_commandBufferPools[i].clear();
    m_commandBufferPools[i].reserve(COMMAND_BUFFERS_PER_FRAME);

    for (uint32_t j = 0; j < INITIAL_COMMAND_BUFFERS; ++j) {
      rhi::CommandBufferDesc cmdDesc;
      auto                   cmdBuffer = m_device->createCommandBuffer(cmdDesc);
      if (cmdBuffer) {
        m_commandBufferPools[i].emplace_back(std::move(cmdBuffer));
      }
    }
  }

  setupRenderPasses_();

  auto deletionManager = ServiceLocator::s_get<ResourceDeletionManager>();
  if (deletionManager) {
    deletionManager->setDefaultFrameDelay(MAX_FRAMES_IN_FLIGHT);
  }

  m_initialized = true;

  GlobalLogger::Log(LogLevel::Info, "Renderer initialized successfully");
  return true;
}

RenderContext Renderer::beginFrame(Scene* scene, const RenderSettings& renderSettings) {
  if (!m_initialized) {
    GlobalLogger::Log(LogLevel::Error, "Renderer not initialized");
    return RenderContext();
  }

  m_currentFrame                = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
  auto& fence                   = m_frameFences[m_currentFrame];
  auto& imageAvailableSemaphore = m_imageAvailableSemaphores[m_currentFrame];
  auto& renderFinishedSemaphore = m_renderFinishedSemaphores[m_currentFrame];

  fence->wait();
  fence->reset();

  auto deletionManager = ServiceLocator::s_get<ResourceDeletionManager>();
  if (deletionManager) {
    deletionManager->setCurrentFrame(m_currentFrame);
  }

  m_resourceManager->updateScheduledPipelines();

  if (!m_swapChain->acquireNextImage(imageAvailableSemaphore.get())) {
    GlobalLogger::Log(LogLevel::Error, "Failed to acquire next swapchain image");
    return RenderContext();
  }

  auto commandBuffer = acquireCommandBuffer_();
  commandBuffer->begin();

#ifdef GAME_ENGINE_RHI_DX12
  if (m_device->getApiType() == rhi::RenderingApi::Dx12) {
    auto dx12CmdBuffer = static_cast<rhi::CommandBufferDx12*>(commandBuffer.get());
    dx12CmdBuffer->bindDescriptorHeaps();
  }
#endif  //  GAME_ENGINE_RHI_DX12

  math::Dimension2Di viewportDimension;
  switch (renderSettings.appMode) {
    case ApplicationRenderMode::Game:
      viewportDimension = m_window->getSize();
      break;
    case ApplicationRenderMode::Editor:
      viewportDimension = renderSettings.renderViewportDimension;
      onViewportResize(viewportDimension);
      break;
    default:
      GlobalLogger::Log(LogLevel::Error, "Invalid application mode");
      return RenderContext();
  }

  auto  swapchainImage    = m_swapChain->getCurrentImage();
  auto& renderTarget      = m_frameResources->getRenderTargets(m_swapChain->getCurrentImageIndex());
  renderTarget.backBuffer = swapchainImage;

  RenderContext context;
  context.scene             = scene;
  context.commandBuffer     = std::move(commandBuffer);
  context.viewportDimension = viewportDimension;
  context.renderSettings    = renderSettings;
  context.currentImageIndex = m_swapChain->getCurrentImageIndex();
  context.waitSemaphore     = imageAvailableSemaphore.get();
  context.signalSemaphore   = renderFinishedSemaphore.get();

  m_frameResources->updatePerFrameResources(context);

  return context;
}

void Renderer::renderFrame(RenderContext& context) {
  if (!context.commandBuffer || !m_frameResources.get()) {
    GlobalLogger::Log(LogLevel::Error, "Invalid render context");
    return;
  }

  // TODO: divide this into separate functions

  if (m_basePass) {
    m_basePass->prepareFrame(context);
  }

  if (m_debugPass) {
    m_debugPass->prepareFrame(context);
  }

  if (m_finalPass) {
    m_finalPass->prepareFrame(context);
  }

  bool exclusiveMode
      = m_debugPass && context.renderSettings.renderMode != RenderMode::Solid && m_debugPass->isExclusive();

  if (!exclusiveMode && m_basePass) {
    m_basePass->render(context);
  }

  auto isDebugPass = context.renderSettings.renderMode == RenderMode::Wireframe
                  || context.renderSettings.renderMode == RenderMode::ShaderOverdraw
                  || context.renderSettings.renderMode == RenderMode::VertexNormalVisualization
                  || context.renderSettings.renderMode == RenderMode::NormalMapVisualization
                  || context.renderSettings.renderMode == RenderMode::LightVisualization;

  if (m_debugPass && isDebugPass) {
    m_debugPass->render(context);
  }

  if (m_finalPass && context.renderSettings.appMode == ApplicationRenderMode::Game) {
    m_finalPass->render(context);
  }

  if (m_basePass) {
    m_basePass->endFrame();
  }

  if (m_debugPass) {
    m_debugPass->endFrame();
  }

  if (m_finalPass) {
    m_finalPass->endFrame();
  }
}

void Renderer::endFrame(RenderContext& context) {
  if (!context.commandBuffer) {
    return;
  }

  context.commandBuffer->end();

  std::vector<rhi::Semaphore*> waitSemaphores;
  if (context.waitSemaphore) {
    waitSemaphores.push_back(context.waitSemaphore);
  }

  std::vector<rhi::Semaphore*> signalSemaphores;
  if (context.signalSemaphore) {
    signalSemaphores.push_back(context.signalSemaphore);
  }

  m_device->submitCommandBuffer(
      context.commandBuffer.get(), m_frameFences[m_currentFrame].get(), waitSemaphores, signalSemaphores);

  m_swapChain->present(context.signalSemaphore);

  recycleCommandBuffer_(std::move(context.commandBuffer));

  // m_frameResources->clearDirtyFlags(context);

  m_frameIndex++;
}

bool Renderer::onWindowResize(uint32_t width, uint32_t height) {
  waitForAllFrames_();

  if (!m_swapChain->resize(width, height)) {
    GlobalLogger::Log(LogLevel::Error, "Failed to resize swap chain");
    return false;
  }

  m_frameResources->resize(math::Dimension2Di(width, height));

  if (m_basePass) {
    m_basePass->resize(math::Dimension2Di(width, height));
  }

  if (m_debugPass) {
    m_debugPass->resize(math::Dimension2Di(width, height));
  }

  if (m_finalPass) {
    m_finalPass->resize(math::Dimension2Di(width, height));
  }

  return true;
}

bool Renderer::onViewportResize(const math::Dimension2Di& newDimension) {
  const auto& currentViewport = m_frameResources->getViewport();

  if (static_cast<int>(currentViewport.width) == newDimension.width()
      && static_cast<int>(currentViewport.height) == newDimension.height()) {
    return true;
  }

  m_device->waitIdle();

  auto width  = newDimension.width() > 0 ? newDimension.width() : 1;
  auto height = newDimension.height() > 0 ? newDimension.height() : 1;

  GlobalLogger::Log(LogLevel::Info, "Resizing viewport to " + std::to_string(width) + "x" + std::to_string(height));

  // this is not ideal solution, but it works for now
  auto scene = ServiceLocator::s_get<SceneManager>()->getCurrentScene();
  if (scene) {
    auto& registry = scene->getEntityRegistry();
    auto  view     = registry.view<Camera>();

    if (!view.empty()) {
      auto  entity  = view.front();
      auto& camera  = view.get<Camera>(entity);
      camera.width  = width;
      camera.height = height;
    }
  }

  m_frameResources->resize(math::Dimension2Di(width, height));

  if (m_basePass) {
    m_basePass->resize(math::Dimension2Di(width, height));
  }

  if (m_debugPass) {
    m_debugPass->resize(math::Dimension2Di(width, height));
  }

  if (m_finalPass) {
    m_finalPass->resize(math::Dimension2Di(width, height));
  }

  return true;
}

std::unique_ptr<rhi::CommandBuffer> Renderer::acquireCommandBuffer_() {
  auto& pool = m_commandBufferPools[m_currentFrame];

  if (!pool.empty()) {
    auto cmdBuffer = std::move(pool.back());
    pool.pop_back();

    cmdBuffer->reset();

    return cmdBuffer;
  }

  rhi::CommandBufferDesc cmdDesc;
  return m_device->createCommandBuffer(cmdDesc);
}

void Renderer::recycleCommandBuffer_(std::unique_ptr<rhi::CommandBuffer> cmdBuffer) {
  if (cmdBuffer) {
    auto& pool = m_commandBufferPools[m_currentFrame];

    if (pool.size() < COMMAND_BUFFERS_PER_FRAME) {
      pool.push_back(std::move(cmdBuffer));
    }
  }
}

void Renderer::waitForAllFrames_() {
  if (m_device) {
    m_device->waitIdle();
  }
}

void Renderer::setupRenderPasses_() {
  m_basePass = std::make_unique<BasePass>();
  m_basePass->initialize(m_device.get(), getResourceManager(), m_frameResources.get(), m_shaderManager.get());

  m_debugPass = std::make_unique<DebugPass>();
  m_debugPass->initialize(m_device.get(), getResourceManager(), m_frameResources.get(), m_shaderManager.get());

  m_finalPass = std::make_unique<FinalPass>();
  m_finalPass->initialize(m_device.get(), getResourceManager(), m_frameResources.get(), m_shaderManager.get());
}
}  // namespace renderer
}  // namespace gfx
}  // namespace game_engine