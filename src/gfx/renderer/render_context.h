#ifndef GAME_ENGINE_RENDER_CONTEXT_H
#define GAME_ENGINE_RENDER_CONTEXT_H

#include "gfx/renderer/render_settings.h"
#include "gfx/rhi/interface/command_buffer.h"
#include "gfx/rhi/interface/synchronization.h"
#include "scene/scene.h"

#include <memory>

namespace game_engine {
namespace gfx {
namespace renderer {

/**
 * Context holding all the information needed for rendering a frame
 */
struct RenderContext {
  Scene*                              scene = nullptr;
  std::unique_ptr<rhi::CommandBuffer> commandBuffer;
  math::Dimension2Di                  viewportDimension;
  RenderSettings                      renderSettings;
  uint32_t                            currentImageIndex = 0;

  // TODO: currently used only in renderer, cosider remove from RenderContext
  rhi::Semaphore* waitSemaphore   = nullptr;
  rhi::Semaphore* signalSemaphore = nullptr;
};

}  // namespace renderer
}  // namespace gfx
}  // namespace game_engine

#endif  // GAME_ENGINE_RENDER_CONTEXT_H