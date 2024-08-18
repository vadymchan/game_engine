
#ifndef GAME_ENGINE_SHADER_BINDABLE_RESOURCE_H
#define GAME_ENGINE_SHADER_BINDABLE_RESOURCE_H

#include "gfx/rhi/name.h"

namespace game_engine {

struct ShaderBindableResource {
  ShaderBindableResource() = default;

  ShaderBindableResource(const Name& name)
      : m_resourceName_(name) {}

  virtual ~ShaderBindableResource() {}

  Name m_resourceName_;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_SHADER_BINDABLE_RESOURCE_H