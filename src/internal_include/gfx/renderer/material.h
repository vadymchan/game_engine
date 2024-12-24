#ifndef GAME_ENGINE_MATERIAL_OLD_H
#define GAME_ENGINE_MATERIAL_OLD_H

#include "gfx/rhi/name.h"
#include "gfx/rhi/shader_binding_layout.h"
#include "gfx/rhi/texture.h"

#include <memory>

namespace game_engine {

class MaterialOld {
  public:
  // ======= BEGIN: public nested types =======================================

  enum class EMaterialTextureType : int8_t {
    Albedo = 0,
    Normal,
    Env,
    Max
  };

  struct TextureData {
    const Texture* getTexture() const { return m_texture_; }

    Name                m_name_;
    Name                m_filePath_;
    Texture*            m_texture_             = nullptr;
    ETextureAddressMode m_textureAddressModeU_ = ETextureAddressMode::REPEAT;
    ETextureAddressMode m_textureAddressModeV_ = ETextureAddressMode::REPEAT;
  };

  // ======= END: public nested types   =======================================

  // ======= BEGIN: public destructor =========================================

  virtual ~MaterialOld() {}

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public getters ============================================

  template <typename T>
  T* getTexture(EMaterialTextureType type) const {
    return (T*)(getTexture(type));
  }

  Texture* getTexture(EMaterialTextureType type) const;

  // ======= END: public getters   ============================================

  // ======= BEGIN: public misc methods =======================================

  bool hasAlbedoTexture() const {
    return m_texData_[(int32_t)EMaterialTextureType::Albedo].m_texture_;
  }

  bool isUseSphericalMap() const { return m_useSphericalMap_; }

  bool isUseSRGBAlbedoTexture() const {
    return m_texData_[(int32_t)EMaterialTextureType::Albedo].m_texture_
             ? m_texData_[(int32_t)EMaterialTextureType::Albedo]
                   .m_texture_->m_sRGB_
             : false;
  }

  const std::shared_ptr<ShaderBindingInstance>& createShaderBindingInstance();

  // ======= END: public misc methods   =======================================

  // ======= BEGIN: public misc fields ========================================

  TextureData m_texData_[static_cast<int32_t>(EMaterialTextureType::Max)];
  bool        m_useSphericalMap_ = false;

  std::shared_ptr<ShaderBindingInstance> m_shaderBindingInstance_ = nullptr;
  mutable bool m_needToUpdateShaderBindingInstance_               = true;

  // ======= END: public misc fields   ========================================
};

}  // namespace game_engine

#endif  // GAME_ENGINE_MATERIAL_OLD_H
