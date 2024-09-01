
#ifndef GAME_ENGINE_SHADER_BINDABLE_RESOURCE_H
#define GAME_ENGINE_SHADER_BINDABLE_RESOURCE_H

#include "gfx/rhi/name.h"

namespace game_engine {

struct ShaderBindableResource {
  // ======= BEGIN: public constructors =======================================

  ShaderBindableResource() = default;

  ShaderBindableResource(const Name& name)
      : m_resourceName_(name) {}

  // ======= END: public constructors   =======================================

  // ======= BEGIN: public destructor =========================================

  virtual ~ShaderBindableResource() {}

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public misc fields ========================================

  Name m_resourceName_;

  // ======= END: public misc fields   ========================================
};

}  // namespace game_engine

#endif  // GAME_ENGINE_SHADER_BINDABLE_RESOURCE_H