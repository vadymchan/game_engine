#ifndef GAME_ENGINE_RENDER_FRAME_CONTEXT_H
#define GAME_ENGINE_RENDER_FRAME_CONTEXT_H

#include "gfx/rhi/command_buffer_manager.h"
#include "gfx/rhi/render_target.h"

#include <memory>

namespace game_engine {

class Scene;
struct RenderFrameContext;

// TODO: move this to a separate file (editor related)
enum class RenderMode {
  Solid,
  Wireframe,
  NormalMapVisualization,
  VertexNormalVisualization,
  ShaderOverdraw,
};

enum class PostProcessMode {
  None,
  Grayscale,
  ColorInversion
};

struct EditorRenderParams {
  RenderMode         renderMode;
  PostProcessMode    postProcessMode;
  // TODO: use value from config
  math::Dimension2Di editorViewportDimension = math::Dimension2Di(1, 1);
};

// TODO: Move this to a separate file
struct RenderContext {
  std::shared_ptr<Scene>              scene;
  std::shared_ptr<RenderFrameContext> renderFrameContext;
  math::Dimension2Di                  viewportDimension;
  EditorRenderParams                  editorRenderParams;
};

struct RenderFrameContext
    : public std::enable_shared_from_this<RenderFrameContext> {
  // ======= BEGIN: public nested types =======================================

  // ======= END: public nested types   =======================================
  enum ECurrentRenderPass {
    None,
    ShadowPass,
    BasePass,
  };

  // ======= BEGIN: public constructors =======================================

  RenderFrameContext() = default;

  RenderFrameContext(std::shared_ptr<CommandBuffer> commandBuffer)
      : m_commandBuffer_(commandBuffer) {}

  // ======= END: public constructors   =======================================

  // ======= BEGIN: public destructor =========================================

  virtual ~RenderFrameContext();

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overridden methods =================================

  virtual void destroy();

  virtual void submitCurrentActiveCommandBuffer(
      ECurrentRenderPass currentRenderPass) {}

  virtual std::shared_ptr<CommandBuffer> getActiveCommandBuffer() const {
    return m_commandBuffer_;
  }

  // ======= END: public overridden methods   =================================

  // ======= BEGIN: public misc fields ========================================

  std::shared_ptr<SceneRenderTarget> m_sceneRenderTarget_ = nullptr;
  uint32_t                           m_frameIndex_        = -1;

  // ======= END: public misc fields   ========================================

  protected:
  // ======= BEGIN: protected misc fields =====================================

  std::shared_ptr<CommandBuffer> m_commandBuffer_ = nullptr;

  // ======= END: protected misc fields   =====================================
};

}  // namespace game_engine

#endif  // GAME_ENGINE_RENDER_FRAME_CONTEXT_H
