#ifndef GAME_ENGINE_MATERIAL_H
#define GAME_ENGINE_MATERIAL_H

#include "gfx/rhi/name.h"
#include "gfx/rhi/vulkan/shader_binding_layout_vk.h"
#include "gfx/rhi/vulkan/texture_vk.h"

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
    Name                 name;
    Name                 FilePath;
    TextureVk*           Texture             = nullptr;
    ETextureAddressMode  TextureAddressModeU = ETextureAddressMode::REPEAT;
    ETextureAddressMode  TextureAddressModeV = ETextureAddressMode::REPEAT;

    const TextureVk* GetTexture() const { return Texture; }
  };

  bool HasAlbedoTexture() const {
    return TexData[(int32_t)EMaterialTextureType::Albedo].Texture;
  }

  bool IsUseSphericalMap() const { return bUseSphericalMap; }

  bool IsUseSRGBAlbedoTexture() const {
    return TexData[(int32_t)EMaterialTextureType::Albedo].Texture
             ? TexData[(int32_t)EMaterialTextureType::Albedo].Texture->sRGB
             : false;
  }

  TextureVk* GetTexture(EMaterialTextureType InType) const;

  template <typename T>
  T* GetTexture(EMaterialTextureType InType) const {
    return (T*)(GetTexture(InType));
  }

  TextureData TexData[static_cast<int32_t>(EMaterialTextureType::Max)];
  bool        bUseSphericalMap = false;

  const std::shared_ptr<ShaderBindingInstance>& CreateShaderBindingInstance();

  std::shared_ptr<ShaderBindingInstance> ShaderBindingInstance = nullptr;
  mutable bool NeedToUpdateShaderBindingInstance               = true;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_MATERIAL_H