#include "gfx/renderer/passes/final_pass.h"

#include "gfx/renderer/frame_resources.h"

namespace game_engine {
namespace gfx {
namespace renderer {

void FinalPass::initialize(rhi::Device*           device,
                           RenderResourceManager* resourceManager,
                           FrameResources*        frameResources,
                           rhi::ShaderManager*    shaderManager) {
  m_device          = device;
  m_resourceManager = resourceManager;
  m_frameResources  = frameResources;
}

void FinalPass::render(const RenderContext& context) {
  auto commandBuffer = context.commandBuffer.get();

  if (!commandBuffer || !m_frameResources) {
    return;
  }

  auto& renderTargets = m_frameResources->getRenderTargets(context.currentImageIndex);

  if (renderTargets.colorBuffer && renderTargets.backBuffer) {
    commandBuffer->copyTexture(renderTargets.colorBuffer.get(), renderTargets.backBuffer);
  }
}

void FinalPass::cleanup() {
  m_device          = nullptr;
  m_resourceManager = nullptr;
}

}  // namespace renderer
}  // namespace gfx
}  // namespace game_engine