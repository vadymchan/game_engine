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

  bool            isOperationAllowedForEntity(ImGuizmo::OPERATION operation);
  void            updateGizmoConstraints();
  math::Vector3Df getGizmoWorldPosition();
  void            handleGizmoManipulation(const math::Matrix4f<>& modelMatrix);
  void            handleEntitySelection(entt::entity entity);

  bool             shouldRenderGizmo_();
  entt::entity     getCameraEntity_();
  void             setupImGuizmo_(const ImVec2& viewportPos, const math::Dimension2Di& viewportSize);
  math::Matrix4f<> calculateEntityModelMatrix_();
  math::Matrix4f<> calculateDirectionalLightMatrix_(Registry& registry);
  bool             performGizmoManipulation_(math::Matrix4f<>& modelMatrix, entt::entity cameraEntity);

  // Editor state
  gfx::renderer::RenderSettings m_renderParams;

  Window*                        m_window         = nullptr;
  gfx::rhi::Device*              m_device         = nullptr;
  gfx::renderer::FrameResources* m_frameResources = nullptr;

  std::unique_ptr<gfx::ImGuiRHIContext> m_imguiContext;
  std::vector<ImTextureID>              m_viewportTextureIDs;

  bool m_pendingViewportResize = true;

  entt::entity m_selectedEntity = entt::null;

  bool                m_showGizmo             = true;
  ImGuizmo::OPERATION m_currentGizmoOperation = ImGuizmo::TRANSLATE;
  ImGuizmo::MODE      m_currentGizmoMode      = ImGuizmo::WORLD;

  math::Vector3Df m_currentDirectionalLightGizmoPosition;
  //bool            m_isDirectionalLightSelected = false;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_EDITOR_H