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
    Name                name;
    Name                FilePath;
    Texture*          m_texture             = nullptr;
    ETextureAddressMode TextureAddressModeU = ETextureAddressMode::REPEAT;
    ETextureAddressMode TextureAddressModeV = ETextureAddressMode::REPEAT;

    const Texture* GetTexture() const { return m_texture; }
  };

  bool HasAlbedoTexture() const {
    return TexData[(int32_t)EMaterialTextureType::Albedo].m_texture;
  }

  bool IsUseSphericalMap() const { return bUseSphericalMap; }

  bool IsUseSRGBAlbedoTexture() const {
    return TexData[(int32_t)EMaterialTextureType::Albedo].m_texture
             ? TexData[(int32_t)EMaterialTextureType::Albedo].m_texture->sRGB
             : false;
  }

  Texture* GetTexture(EMaterialTextureType InType) const;

  template <typename T>
  T* GetTexture(EMaterialTextureType InType) const {
    return (T*)(GetTexture(InType));
  }

  TextureData TexData[static_cast<int32_t>(EMaterialTextureType::Max)];
  bool        bUseSphericalMap = false;

  const std::shared_ptr<ShaderBindingInstance>& CreateShaderBindingInstance();

  std::shared_ptr<ShaderBindingInstance> m_shaderBindingInstance = nullptr;
  mutable bool NeedToUpdateShaderBindingInstance                 = true;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_MATERIAL_H