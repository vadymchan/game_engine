#ifndef GAME_ENGINE_SHADER_RELOAD_OBSERVER_H
#define GAME_ENGINE_SHADER_RELOAD_OBSERVER_H

namespace game_engine {
namespace gfx {
namespace rhi {

class Shader;

/**
 * Interface for objects that need to be notified when a shader is reloaded
 */
class ShaderReloadObserver {
  public:
  virtual ~ShaderReloadObserver() = default;

  virtual void onShaderReloaded(Shader* shader) = 0;
};

}  // namespace rhi
}  // namespace gfx
}  // namespace game_engine

#endif  // GAME_ENGINE_SHADER_RELOAD_OBSERVER_H