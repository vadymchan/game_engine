// In editor.h
#ifndef GAME_ENGINE_EDITOR_H
#define GAME_ENGINE_EDITOR_H

#include "gfx/renderer/renderer.h"
#include "gfx/rhi/common/rhi_enums.h"
#include "utils/ui/imgui_rhi_context.h" 

#include <ImGuizmo.h>

namespace game_engine {

class Window;

namespace gfx::rhi {
class Device;
}  // namespace gfx::rhi

class Editor {
  public:
  Editor()  = default;
  ~Editor() = default;

  bool initialize(Window*                        window,
                  gfx::rhi::RenderingApi         renderingApi,
                  gfx::rhi::Device*              device,
                  gfx::renderer::FrameResources* frameResources);

  void render(gfx::renderer::RenderContext& context);

  const gfx::renderer::RenderSettings& getRenderParams() const { return m_renderParams; }

  void onWindowResize(uint32_t width, uint32_t height);

  private:
  void resizeViewport(const gfx::renderer::RenderContext& context);

  void renderPerformanceWindow();
  void renderViewportWindow(gfx::renderer::RenderContext& context);
  void renderModeSelectionWindow();
  void renderSceneHierarchyWindow();
  void renderInspectorWindow();

  void renderGizmo(const math::Dimension2Di& viewportSize, const ImVec2& viewportPos);
  void handleGizmoInput();
  void renderGizmoControlsWindow();


  Window*                        m_window         = nullptr;
  gfx::rhi::Device*              m_device         = nullptr;
  gfx::renderer::FrameResources* m_frameResources = nullptr;

  std::unique_ptr<gfx::ImGuiRHIContext> m_imguiContext;
  std::vector<ImTextureID>              m_viewportTextureIDs;

  bool               m_pendingViewportResize = true;

  entt::entity m_selectedEntity = entt::null;

  bool                m_showGizmo             = true;
  ImGuizmo::OPERATION m_currentGizmoOperation = ImGuizmo::TRANSLATE;
  ImGuizmo::MODE      m_currentGizmoMode      = ImGuizmo::WORLD;

  // Editor state
  gfx::renderer::RenderSettings m_renderParams;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_EDITOR_H