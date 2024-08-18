#ifndef GAME_ENGINE_MATERIAL_H
#define GAME_ENGINE_MATERIAL_H

#include "gfx/rhi/name.h"
#include "gfx/rhi/shader_binding_layout.h"
#include "gfx/rhi/texture.h"

#include <memory>

namespace game_engine {

class Material {
  public:
  virtual ~Material() {}

  enum class EMaterialTextureType : int8_t {
    Albedo = 0,
    Normal,
    Env,
    Max
  };

  struct TextureData {
    const Texture* GetTexture() const { return m_texture_; }

    Name                m_name_;
    Name                m_filePath_;
    Texture*            m_texture_           = nullptr;
    ETextureAddressMode m_textureAddressModeU_ = ETextureAddressMode::REPEAT;
    ETextureAddressMode m_textureAddressModeV_ = ETextureAddressMode::REPEAT;
  };

  bool HasAlbedoTexture() const {
    return m_texData_[(int32_t)EMaterialTextureType::Albedo].m_texture_;
  }

  bool IsUseSphericalMap() const { return m_useSphericalMap_; }

  bool IsUseSRGBAlbedoTexture() const {
    return m_texData_[(int32_t)EMaterialTextureType::Albedo].m_texture_
             ? m_texData_[(int32_t)EMaterialTextureType::Albedo].m_texture_->m_sRGB_
             : false;
  }

  Texture* GetTexture(EMaterialTextureType type) const;

  template <typename T>
  T* GetTexture(EMaterialTextureType type) const {
    return (T*)(GetTexture(type));
  }

  TextureData m_texData_[static_cast<int32_t>(EMaterialTextureType::Max)];
  bool        m_useSphericalMap_ = false;

  const std::shared_ptr<ShaderBindingInstance>& CreateShaderBindingInstance();

  std::shared_ptr<ShaderBindingInstance> m_shaderBindingInstance_ = nullptr;
  mutable bool m_needToUpdateShaderBindingInstance_                 = true;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_MATERIAL_H