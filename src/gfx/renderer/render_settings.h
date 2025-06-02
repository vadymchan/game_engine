#ifndef ARISE_RENDER_SETTINGS_H
#define ARISE_RENDER_SETTINGS_H

#include <math_library/dimension.h>

namespace arise {
namespace gfx {
namespace renderer {

enum class ApplicationRenderMode {
  Editor,
  Game
};

enum class RenderMode {
  Solid,
  Wireframe,
  NormalMapVisualization,
  VertexNormalVisualization,
  ShaderOverdraw,
  LightVisualization,
  WorldGrid,
  MeshHighlight,
};

enum class PostProcessMode {
  None,
  Grayscale,
  ColorInversion
};

struct RenderSettings {
  RenderMode            renderMode              = RenderMode::Solid;
  PostProcessMode       postProcessMode         = PostProcessMode::None;
  math::Dimension2Di    renderViewportDimension = math::Dimension2Di(1, 1);
  ApplicationRenderMode appMode                 = ApplicationRenderMode::Game;
};

}  // namespace renderer
}  // namespace gfx
}  // namespace arise

#endif  // ARISE_RENDER_SETTINGS_H