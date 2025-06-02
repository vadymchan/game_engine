#ifndef ARISE_DEBUG_DRAW_STRATEGY_H
#define ARISE_DEBUG_DRAW_STRATEGY_H

#include "gfx/renderer/render_context.h"
#include "gfx/rhi/interface/device.h"

#include <math_library/dimension.h>

namespace arise::gfx::rhi {
class ShaderManager;
}  // namespace arise::gfx::rhi

namespace arise {
namespace gfx {
namespace renderer {

class RenderResourceManager;
class FrameResources;

/**
 * Base class for debug visualization strategies
 */
class DebugDrawStrategy {
  public:
  DebugDrawStrategy()          = default;
  virtual ~DebugDrawStrategy() = default;

  DebugDrawStrategy(const DebugDrawStrategy&)            = delete;
  DebugDrawStrategy& operator=(const DebugDrawStrategy&) = delete;
  DebugDrawStrategy(DebugDrawStrategy&&)                 = delete;
  DebugDrawStrategy& operator=(DebugDrawStrategy&&)      = delete;

  virtual void initialize(rhi::Device*           device,
                          RenderResourceManager* resourceManager,
                          FrameResources*        frameResources,
                          rhi::ShaderManager*    shaderManager)
      = 0;

  virtual void resize(const math::Dimension2Di& newDimension) = 0;
  virtual void prepareFrame(const RenderContext& context) = 0;
  virtual void render(const RenderContext& context) = 0;
  virtual void cleanup() = 0;

  virtual bool isExclusive() const { return false; }
};

}  // namespace renderer
}  // namespace gfx
}  // namespace arise

#endif  // ARISE_DEBUG_DRAW_STRATEGY_H