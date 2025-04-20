#include "editor/editor.h"

#include "config/config_manager.h"
#include "ecs/components/camera.h"
#include "ecs/components/light.h"
#include "ecs/components/render_model.h"
#include "scene/scene_manager.h"
#include "utils/service/service_locator.h"
#include "utils/time/timing_manager.h"

namespace game_engine {

bool Editor::initialize(Window*                        window,
                        gfx::rhi::RenderingApi         renderingApi,
                        gfx::rhi::Device*              device,
                        gfx::renderer::FrameResources* frameResources) {
  m_window         = window;
  m_device         = device;
  m_frameResources = frameResources;
  m_viewportTextureIDs.resize(frameResources->getFramesCount(), 0);

  m_imguiContext = std::make_unique<gfx::ImGuiRHIContext>();
  if (!m_imguiContext->initialize(window, device, frameResources->getFramesCount())) {
    GlobalLogger::Log(LogLevel::Error, "Failed to initialize ImGui context");
    return false;
  }

  ImGuizmo::SetImGuiContext(ImGui::GetCurrentContext());

  m_showGizmo             = true;
  m_currentGizmoOperation = ImGuizmo::TRANSLATE;
  m_currentGizmoMode      = ImGuizmo::WORLD;

  m_renderParams.renderMode              = gfx::renderer::RenderMode::Solid;
  m_renderParams.postProcessMode         = gfx::renderer::PostProcessMode::None;
  m_renderParams.appMode                 = gfx::renderer::ApplicationRenderMode::Editor;
  m_renderParams.renderViewportDimension = window->getSize();

  GlobalLogger::Log(LogLevel::Info, "Editor initialized successfully");
  return true;
}

void Editor::render(gfx::renderer::RenderContext& context) {
  if (!m_imguiContext) {
    return;
  }

  if (m_pendingViewportResize) {
    resizeViewport(context);
    m_pendingViewportResize = false;
  }

  uint32_t currentIndex = context.currentImageIndex;

  auto colorBufferTexture = m_frameResources->getRenderTargets(currentIndex).colorBuffer.get();

  if (colorBufferTexture->getCurrentLayoutType() != gfx::rhi::ResourceLayout::ShaderReadOnly) {
    gfx::rhi::ResourceBarrierDesc colorBarrier;
    colorBarrier.texture   = colorBufferTexture;
    colorBarrier.oldLayout = colorBufferTexture->getCurrentLayoutType();
    colorBarrier.newLayout = gfx::rhi::ResourceLayout::ShaderReadOnly;
    context.commandBuffer->resourceBarrier(colorBarrier);
  }

  if (!m_viewportTextureIDs[currentIndex]) {
    m_viewportTextureIDs[currentIndex] = m_imguiContext->createTextureID(colorBufferTexture, currentIndex);
  }

  m_imguiContext->beginFrame();

  ImGuizmo::BeginFrame();

  ImGui::DockSpaceOverViewport();

  renderPerformanceWindow();
  renderViewportWindow(context);
  renderModeSelectionWindow();
  renderSceneHierarchyWindow();
  renderInspectorWindow();
  renderGizmoControlsWindow();

  auto backBufferTexture = m_frameResources->getRenderTargets(context.currentImageIndex).backBuffer;

  m_imguiContext->endFrame(
      context.commandBuffer.get(), backBufferTexture, m_window->getSize(), context.currentImageIndex);
}

void Editor::onWindowResize(uint32_t width, uint32_t height) {
  if (m_renderParams.appMode == gfx::renderer::ApplicationRenderMode::Editor) {
    if (width <= 0 || height <= 0) {
      GlobalLogger::Log(LogLevel::Error, "Invalid window dimensions");
      return;
    }

    math::Dimension2Di newDimensions(width, height);

    m_imguiContext->resize(newDimensions);

    m_pendingViewportResize = true;
  }
}

void Editor::resizeViewport(const gfx::renderer::RenderContext& context) {
  if (m_renderParams.renderViewportDimension.width() <= 0 || m_renderParams.renderViewportDimension.height() <= 0) {
    GlobalLogger::Log(LogLevel::Error, "Invalid viewport dimensions");
    return;
  }

  m_device->waitIdle();

  for (size_t i = 0; i < m_viewportTextureIDs.size(); i++) {
    if (m_viewportTextureIDs[i]) {
      m_imguiContext->releaseTextureID(m_viewportTextureIDs[i]);
      m_viewportTextureIDs[i] = 0;
    }
  }

  // We'll create new texture IDs on-demand in the render method
}

void Editor::renderPerformanceWindow() {
  auto timingManager = ServiceLocator::s_get<TimingManager>();
  if (!timingManager) {
    return;
  }

  ImGui::Begin("Performance");

  auto fps       = timingManager->getFPS();
  auto frameTime = timingManager->getFrameTime();

  ImGui::Text("FPS: %.1f", fps);
  ImGui::Text("Frame Time: %.1f ms", frameTime);

  // Frame time history graph
  static const int historyCount                   = 300;
  static float     frameTimeHistory[historyCount] = {0};
  static int       historyIndex                   = 0;

  frameTimeHistory[historyIndex] = frameTime;
  historyIndex                   = (historyIndex + 1) % historyCount;

  ImGui::PlotLines(
      "Frame Time (ms)", frameTimeHistory, historyCount, historyIndex, nullptr, 0.0f, FLT_MAX, ImVec2(0, 80));

  ImGui::End();
}

void Editor::renderViewportWindow(gfx::renderer::RenderContext& context) {
  ImGui::Begin("Render Window");

  ImVec2 renderWindow = ImGui::GetContentRegionAvail();

  if (renderWindow.x <= 0.0f) {
    renderWindow.x = 1.0f;
  }
  if (renderWindow.y <= 0.0f) {
    renderWindow.y = 1.0f;
  }

  math::Dimension2Di newDimension(renderWindow.x, renderWindow.y);

  bool viewportResized = (m_renderParams.renderViewportDimension.width() != static_cast<int>(renderWindow.x)
                          || m_renderParams.renderViewportDimension.height() != static_cast<int>(renderWindow.y));

  if (viewportResized) {
    m_renderParams.renderViewportDimension = newDimension;
    m_pendingViewportResize                = true;
  }

  uint32_t    currentIndex     = context.currentImageIndex;
  ImTextureID currentTextureID = m_viewportTextureIDs[currentIndex];

  ImVec2 viewportPos = ImGui::GetCursorScreenPos();

  if (currentTextureID) {
    ImGui::Image(currentTextureID, renderWindow);

    handleGizmoInput();

    renderGizmo(newDimension, viewportPos);

    if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && !ImGuizmo::IsOver()) {
      GlobalLogger::Log(LogLevel::Info, "Viewport clicked, but not over gizmo");
    }
  } else {
    ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Render texture not available");
  }

  ImGui::End();
}

void Editor::renderModeSelectionWindow() {
  ImGui::Begin("Render Mode");

  if (ImGui::RadioButton("Solid", m_renderParams.renderMode == gfx::renderer::RenderMode::Solid)) {
    m_renderParams.renderMode = gfx::renderer::RenderMode::Solid;
  }

  if (ImGui::RadioButton("Wireframe", m_renderParams.renderMode == gfx::renderer::RenderMode::Wireframe)) {
    m_renderParams.renderMode = gfx::renderer::RenderMode::Wireframe;
  }

  if (ImGui::RadioButton("Normal Map Visualization",
                         m_renderParams.renderMode == gfx::renderer::RenderMode::NormalMapVisualization)) {
    m_renderParams.renderMode = gfx::renderer::RenderMode::NormalMapVisualization;
  }

  if (ImGui::RadioButton("Vertex Normal Visualization",
                         m_renderParams.renderMode == gfx::renderer::RenderMode::VertexNormalVisualization)) {
    m_renderParams.renderMode = gfx::renderer::RenderMode::VertexNormalVisualization;
  }

  if (ImGui::RadioButton("Shader Overdraw", m_renderParams.renderMode == gfx::renderer::RenderMode::ShaderOverdraw)) {
    m_renderParams.renderMode = gfx::renderer::RenderMode::ShaderOverdraw;
  }

  ImGui::End();
}

void Editor::renderSceneHierarchyWindow() {
  ImGui::Begin("Scene Hierarchy");

  auto sceneManager = ServiceLocator::s_get<SceneManager>();
  auto scene        = sceneManager->getCurrentScene();

  if (scene) {
    auto& registry = scene->getEntityRegistry();

    ImGui::Text("Current Scene: %s", sceneManager->getCurrentSceneName().c_str());
    ImGui::Separator();

    auto view = registry.view<Transform>();

    for (auto entity : view) {
      std::string label = "Entity " + std::to_string(static_cast<uint32_t>(entity));

      if (registry.all_of<RenderModel*>(entity)) {
        auto* model = registry.get<RenderModel*>(entity);
        if (model && !model->filePath.empty()) {
          label += " (" + model->filePath.filename().string() + ")";
        }
      } else if (registry.all_of<Camera>(entity)) {
        label += " (Camera)";
      } else if (registry.all_of<Light, DirectionalLight>(entity)) {
        label += " (Directional Light)";
      } else if (registry.all_of<Light, PointLight>(entity)) {
        label += " (Point Light)";
      } else if (registry.all_of<Light, SpotLight>(entity)) {
        label += " (Spot Light)";
      }

      bool isSelected = (entity == m_selectedEntity);

      if (ImGui::Selectable(label.c_str(), isSelected)) {
        m_selectedEntity = entity;
      }
    }
  } else {
    ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.5f, 1.0f), "No scene is currently loaded");
  }

  ImGui::End();
}

void Editor::renderInspectorWindow() {
  ImGui::Begin("Inspector");

  if (m_selectedEntity == entt::null) {
    ImGui::Text("No entity selected");
    ImGui::End();
    return;
  }

  auto sceneManager = ServiceLocator::s_get<SceneManager>();
  auto scene        = sceneManager->getCurrentScene();

  if (!scene) {
    ImGui::Text("No scene is currently loaded");
    ImGui::End();
    return;
  }

  auto& registry = scene->getEntityRegistry();

  if (!registry.valid(m_selectedEntity)) {
    ImGui::Text("Selected entity is no longer valid");
    m_selectedEntity = entt::null;
    ImGui::End();
    return;
  }

  ImGui::Text("Entity ID: %u", static_cast<uint32_t>(m_selectedEntity));
  ImGui::Separator();

  // Transform
  if (registry.all_of<Transform>(m_selectedEntity)) {
    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
      auto& transform = registry.get<Transform>(m_selectedEntity);

      float position[3] = {transform.translation.x(), transform.translation.y(), transform.translation.z()};
      if (ImGui::DragFloat3("Position", position, 0.1f)) {
        transform.translation.x() = position[0];
        transform.translation.y() = position[1];
        transform.translation.z() = position[2];
        transform.isDirty         = true;
      }

      float rotation[3] = {transform.rotation.x(), transform.rotation.y(), transform.rotation.z()};
      if (ImGui::DragFloat3("Rotation", rotation, 1.0f)) {
        transform.rotation.x() = rotation[0];
        transform.rotation.y() = rotation[1];
        transform.rotation.z() = rotation[2];
        transform.isDirty      = true;
      }

      float scale[3] = {transform.scale.x(), transform.scale.y(), transform.scale.z()};
      if (ImGui::DragFloat3("Scale", scale, 0.1f)) {
        transform.scale.x() = scale[0];
        transform.scale.y() = scale[1];
        transform.scale.z() = scale[2];
        transform.isDirty   = true;
      }
    }
  }

  // Camera
  if (registry.all_of<Camera>(m_selectedEntity)) {
    if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
      auto& camera = registry.get<Camera>(m_selectedEntity);

      const char* typeItems[] = {"Perspective", "Orthographic"};
      int         currentType = static_cast<int>(camera.type);
      if (ImGui::Combo("Type", &currentType, typeItems, IM_ARRAYSIZE(typeItems))) {
        camera.type = static_cast<CameraType>(currentType);
      }

      if (camera.type == CameraType::Perspective) {
        float fov = camera.fov;
        if (ImGui::SliderFloat("FOV", &fov, 10.0f, 120.0f, "%.1f")) {
          camera.fov = fov;
        }
      }

      float nearClip = camera.nearClip;
      if (ImGui::DragFloat("Near Clip", &nearClip, 0.1f, 0.001f, camera.farClip - 0.001f)) {
        camera.nearClip = nearClip;
      }

      float farClip = camera.farClip;
      if (ImGui::DragFloat("Far Clip", &farClip, 1.0f, camera.nearClip + 0.001f, 10000.0f)) {
        camera.farClip = farClip;
      }

      float width = camera.width;
      if (ImGui::DragFloat("Width", &width, 1.0f, 1.0f, 10000.0f)) {
        camera.width = width;
      }

      float height = camera.height;
      if (ImGui::DragFloat("Height", &height, 1.0f, 1.0f, 10000.0f)) {
        camera.height = height;
      }
    }
  }

  // Light
  if (registry.all_of<Light>(m_selectedEntity)) {
    if (ImGui::CollapsingHeader("Light", ImGuiTreeNodeFlags_DefaultOpen)) {
      auto& light = registry.get<Light>(m_selectedEntity);

      float color[3] = {light.color.x(), light.color.y(), light.color.z()};
      if (ImGui::ColorEdit3("Color", color)) {
        light.color.x() = color[0];
        light.color.y() = color[1];
        light.color.z() = color[2];
      }

      float intensity = light.intensity;
      if (ImGui::DragFloat("Intensity", &intensity, 0.1f, 0.0f, 100.0f)) {
        light.intensity = intensity;
      }

      if (registry.all_of<DirectionalLight>(m_selectedEntity)) {
        auto& dirLight = registry.get<DirectionalLight>(m_selectedEntity);

        float direction[3] = {dirLight.direction.x(), dirLight.direction.y(), dirLight.direction.z()};
        if (ImGui::DragFloat3("Direction", direction, 0.1f)) {
          dirLight.direction.x() = direction[0];
          dirLight.direction.y() = direction[1];
          dirLight.direction.z() = direction[2];
        }
      } else if (registry.all_of<PointLight>(m_selectedEntity)) {
        auto& pointLight = registry.get<PointLight>(m_selectedEntity);

        float range = pointLight.range;
        if (ImGui::DragFloat("Range", &range, 0.1f, 0.0f, 1000.0f)) {
          pointLight.range = range;
        }
      } else if (registry.all_of<SpotLight>(m_selectedEntity)) {
        auto& spotLight = registry.get<SpotLight>(m_selectedEntity);

        float range = spotLight.range;
        if (ImGui::DragFloat("Range", &range, 0.1f, 0.0f, 1000.0f)) {
          spotLight.range = range;
        }

        float innerAngle = spotLight.innerConeAngle;
        if (ImGui::DragFloat("Inner Cone Angle", &innerAngle, 0.1f, 0.0f, spotLight.outerConeAngle)) {
          spotLight.innerConeAngle = innerAngle;
        }

        float outerAngle = spotLight.outerConeAngle;
        if (ImGui::DragFloat("Outer Cone Angle", &outerAngle, 0.1f, spotLight.innerConeAngle, 90.0f)) {
          spotLight.outerConeAngle = outerAngle;
        }
      }
    }
  }

  if (registry.all_of<RenderModel*>(m_selectedEntity)) {
    if (ImGui::CollapsingHeader("Model", ImGuiTreeNodeFlags_DefaultOpen)) {
      auto* model = registry.get<RenderModel*>(m_selectedEntity);

      if (model) {
        ImGui::Text("File: %s", model->filePath.string().c_str());
        ImGui::Text("Meshes: %zu", model->renderMeshes.size());

        for (size_t i = 0; i < model->renderMeshes.size(); i++) {
          if (ImGui::TreeNode(("Mesh " + std::to_string(i)).c_str())) {
            auto* renderMesh = model->renderMeshes[i];

            if (renderMesh && renderMesh->material) {
              ImGui::Text("Material: %s", renderMesh->material->materialName.c_str());

              if (!renderMesh->material->textures.empty() && ImGui::TreeNode("Textures")) {
                for (const auto& [name, texture] : renderMesh->material->textures) {
                  ImGui::Text("%s: %p", name.c_str(), texture);
                }
                ImGui::TreePop();
              }

              if (!renderMesh->material->scalarParameters.empty() && ImGui::TreeNode("Scalar Parameters")) {
                for (auto& [name, value] : renderMesh->material->scalarParameters) {
                  if (ImGui::DragFloat(name.c_str(), &value, 0.01f)) {
                  }
                }
                ImGui::TreePop();
              }

              if (!renderMesh->material->vectorParameters.empty() && ImGui::TreeNode("Vector Parameters")) {
                for (auto& [name, value] : renderMesh->material->vectorParameters) {
                  float values[4] = {value.x(), value.y(), value.z(), value.w()};
                  if (ImGui::DragFloat4(name.c_str(), values, 0.01f)) {
                    value.x() = values[0];
                    value.y() = values[1];
                    value.z() = values[2];
                    value.w() = values[3];
                  }
                }
                ImGui::TreePop();
              }
            } else {
              ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.5f, 1.0f), "No material assigned");
            }

            ImGui::TreePop();
          }
        }
      } else {
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.5f, 1.0f), "Model is null");
      }
    }
  }

  ImGui::End();
}

void Editor::renderGizmo(const math::Dimension2Di& viewportSize, const ImVec2& viewportPos) {
  if (!m_showGizmo || m_selectedEntity == entt::null || !m_frameResources) {
    return;
  }

  auto* sceneManager = ServiceLocator::s_get<SceneManager>();
  auto* scene        = sceneManager->getCurrentScene();

  if (!scene) {
    return;
  }

  auto& registry = scene->getEntityRegistry();

  if (!registry.all_of<Transform>(m_selectedEntity)) {
    return;
  }

  auto& transform = registry.get<Transform>(m_selectedEntity);

  math::Matrix4f<> modelMatrix = calculateTransformMatrix(transform);

  auto cameraView = registry.view<Camera, CameraMatrices>();
  if (cameraView.begin() == cameraView.end()) {
    return;
  }

  auto        cameraEntity   = cameraView.front();
  const auto& cameraMatrices = registry.get<CameraMatrices>(cameraEntity);

  ImGuizmo::SetOrthographic(false);
  ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());

  ImGuizmo::SetRect(viewportPos.x,
                    viewportPos.y,
                    static_cast<float>(viewportSize.width()),
                    static_cast<float>(viewportSize.height()));

  float* viewMatrix      = const_cast<float*>(cameraMatrices.view.data());
  float* projMatrix      = const_cast<float*>(cameraMatrices.projection.data());
  float* modelMatrix_ptr = const_cast<float*>(modelMatrix.data());

  bool manipulated
      = ImGuizmo::Manipulate(viewMatrix, projMatrix, m_currentGizmoOperation, m_currentGizmoMode, modelMatrix_ptr);

  if (manipulated) {
    math::Vector3Df translation, rotation, scale;

    float translationArray[3], rotationArray[3], scaleArray[3];

    ImGuizmo::DecomposeMatrixToComponents(modelMatrix_ptr, translationArray, rotationArray, scaleArray);

    for (int i = 0; i < 3; i++) {
      translation(i) = translationArray[i];
      rotation(i)    = rotationArray[i];
      scale(i)       = scaleArray[i];
    }

    transform.translation = translation;
    transform.rotation    = rotation;
    transform.scale       = scale;
    transform.isDirty     = true;

    GlobalLogger::Log(
        LogLevel::Info,
        "Transform marked as dirty for entity: " + std::to_string(static_cast<uint32_t>(m_selectedEntity)));
  }
}

void Editor::handleGizmoInput() {
  ImGuiIO& io = ImGui::GetIO();
  if (io.WantCaptureKeyboard) {
    return;
  }

  if (ImGui::IsKeyPressed(ImGuiKey_1)) {
    m_currentGizmoOperation = ImGuizmo::TRANSLATE;
    GlobalLogger::Log(LogLevel::Info, "Gizmo: Switched to Translate mode");
  } else if (ImGui::IsKeyPressed(ImGuiKey_2)) {
    m_currentGizmoOperation = ImGuizmo::ROTATE;
    GlobalLogger::Log(LogLevel::Info, "Gizmo: Switched to Rotate mode");
  } else if (ImGui::IsKeyPressed(ImGuiKey_3)) {
    m_currentGizmoOperation = ImGuizmo::SCALE;
    GlobalLogger::Log(LogLevel::Info, "Gizmo: Switched to Scale mode");
  }

  if (ImGui::IsKeyPressed(ImGuiKey_4)) {
    m_currentGizmoMode = (m_currentGizmoMode == ImGuizmo::WORLD) ? ImGuizmo::LOCAL : ImGuizmo::WORLD;
    GlobalLogger::Log(
        LogLevel::Info,
        m_currentGizmoMode == ImGuizmo::WORLD ? "Gizmo: Switched to World space" : "Gizmo: Switched to Local space");
  }

  if (ImGui::IsKeyPressed(ImGuiKey_5)) {
    m_showGizmo = !m_showGizmo;
    GlobalLogger::Log(LogLevel::Info, m_showGizmo ? "Gizmo: Enabled" : "Gizmo: Disabled");
  }
}

void Editor::renderGizmoControlsWindow() {
  ImGui::Begin("Gizmo Controls");

  if (m_selectedEntity == entt::null) {
    ImGui::Text("No entity selected");
    ImGui::End();
    return;
  }

  ImGui::Checkbox("Show Gizmo", &m_showGizmo);

  ImGui::SameLine();
  if (ImGui::Button("T")) {
    m_currentGizmoOperation = ImGuizmo::TRANSLATE;
  }
  ImGui::SameLine();
  if (ImGui::Button("R")) {
    m_currentGizmoOperation = ImGuizmo::ROTATE;
  }
  ImGui::SameLine();
  if (ImGui::Button("S")) {
    m_currentGizmoOperation = ImGuizmo::SCALE;
  }

  const char* modes[]      = {"World", "Local"};
  int         current_mode = (m_currentGizmoMode == ImGuizmo::WORLD) ? 0 : 1;
  if (ImGui::Combo("Space", &current_mode, modes, IM_ARRAYSIZE(modes))) {
    m_currentGizmoMode = (current_mode == 0) ? ImGuizmo::WORLD : ImGuizmo::LOCAL;
  }

  ImGui::Separator();
  ImGui::Text("Keyboard Shortcuts:");

  ImGui::BulletText("1: Translate");
  ImGui::BulletText("2: Rotate");
  ImGui::BulletText("3: Scale");
  ImGui::BulletText("4: Toggle World/Local");
  ImGui::BulletText("5: Toggle Visibility");

  auto* sceneManager = ServiceLocator::s_get<SceneManager>();
  auto* scene        = sceneManager->getCurrentScene();

  if (scene && scene->getEntityRegistry().valid(m_selectedEntity)) {
    ImGui::Separator();
    ImGui::Text("Selected Entity: %u", static_cast<uint32_t>(m_selectedEntity));
  }

  ImGui::End();
}

}  // namespace game_engine