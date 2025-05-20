#include "gfx/renderer/passes/debug_pass.h"

#include "gfx/renderer/debug_strategies/light_visualization_strategy.h"
#include "gfx/renderer/debug_strategies/normal_map_visualization_strategy.h"
#include "gfx/renderer/debug_strategies/shader_overdraw_strategy.h"
#include "gfx/renderer/debug_strategies/vertex_normal_visualization_strategy.h"
#include "gfx/renderer/debug_strategies/wireframe_strategy.h"

namespace game_engine {
namespace gfx {
namespace renderer {

DebugPass::DebugPass() = default;

DebugPass::~DebugPass() {
  cleanup();
}

void DebugPass::initialize(rhi::Device*           device,
                           RenderResourceManager* resourceManager,
                           FrameResources*        frameResources,
                           rhi::ShaderManager*    shaderManager) {
  m_device          = device;
  m_resourceManager = resourceManager;
  m_frameResources  = frameResources;
  m_shaderManager   = shaderManager;
}

void DebugPass::resize(const math::Dimension2Di& newDimension) {
  if (m_debugStrategy) {
    m_debugStrategy->resize(newDimension);
  }
}

void DebugPass::prepareFrame(const RenderContext& context) {
  if (m_currentRenderMode != context.renderSettings.renderMode) {
    if (m_debugStrategy) {
      m_debugStrategy->cleanup();
      m_debugStrategy.reset();
    }

    m_currentRenderMode = context.renderSettings.renderMode;
    createDebugStrategy_();

    if (m_debugStrategy) {
      m_debugStrategy->initialize(m_device, m_resourceManager, m_frameResources, m_shaderManager);
      m_debugStrategy->resize(context.viewportDimension);
    }
  }

  if (m_debugStrategy) {
    m_debugStrategy->prepareFrame(context);
  }
}

void DebugPass::render(const RenderContext& context) {
  if (m_debugStrategy) {
    m_debugStrategy->render(context);
  }
}

void DebugPass::cleanup() {
  if (m_debugStrategy) {
    m_debugStrategy->cleanup();
    m_debugStrategy.reset();
  }
}

bool DebugPass::isExclusive() const {
  return m_debugStrategy ? m_debugStrategy->isExclusive() : false;
}

void DebugPass::createDebugStrategy_() {
  switch (m_currentRenderMode) {
    case RenderMode::Wireframe:
      m_debugStrategy = std::make_unique<WireframeStrategy>();
      break;
    case RenderMode::NormalMapVisualization:
      m_debugStrategy = std::make_unique<NormalMapVisualizationStrategy>();
      break;
    case RenderMode::VertexNormalVisualization:
      m_debugStrategy = std::make_unique<VertexNormalVisualizationStrategy>();
      break;
    case RenderMode::ShaderOverdraw:
      m_debugStrategy = std::make_unique<ShaderOverdrawStrategy>();
      break;
    case RenderMode::LightVisualization:
      m_debugStrategy = std::make_unique<LightVisualizationStrategy>();
      break;
    default:
      // No debug visualization for other render modes
      break;
  }
}

}  // namespace renderer
}  // namespace gfx
}  // namespace game_engine