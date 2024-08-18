
#include "gfx/renderer/material.h"

#include "gfx/rhi/rhi.h"

namespace game_engine {

Texture* Material::GetTexture(EMaterialTextureType inType) const {
  assert(EMaterialTextureType::Albedo <= inType);
  assert(EMaterialTextureType::Max > inType);

  if (!m_texData_[(int32_t)inType].m_texture_) {
    if (inType == EMaterialTextureType::Normal) {
      return g_normalTexture.get();
    }
    return g_blackTexture.get();
  }

  return m_texData_[(int32_t)inType].m_texture_;
}

const std::shared_ptr<ShaderBindingInstance>&
    Material::CreateShaderBindingInstance() {
  if (m_needToUpdateShaderBindingInstance_) {
    m_needToUpdateShaderBindingInstance_ = false;

    int32_t                              BindingPoint = 0;
    ShaderBindingArray                   shaderBindingArray;
    ShaderBindingResourceInlineAllocator ResourceInlineAllactor;

    for (int32_t i = 0; i < (int32_t)EMaterialTextureType::Max; ++i) {
      const TextureData& TextureData = m_texData_[i];
      const Texture*     texture     = TextureData.GetTexture();

      if (!texture) {
        if ((int32_t)EMaterialTextureType::Normal == i) {
          texture = g_normalTexture.get();
        } else if ((int32_t)EMaterialTextureType::Env == i) {
          texture = g_whiteCubeTexture.get();
        } else {
          texture = g_whiteTexture.get();
        }
      }

      shaderBindingArray.Add(ShaderBinding(
          BindingPoint++,
          1,
          EShaderBindingType::TEXTURE_SAMPLER_SRV,
          false,
          EShaderAccessStageFlag::ALL_GRAPHICS,
          ResourceInlineAllactor.Alloc<TextureResource>(texture, nullptr)));
    }

    if (m_shaderBindingInstance_) {
      m_shaderBindingInstance_->Free();
    }

    m_shaderBindingInstance_ = g_rhi->CreateShaderBindingInstance(
        shaderBindingArray, ShaderBindingInstanceType::MultiFrame);
  }
  return m_shaderBindingInstance_;
}

}  // namespace game_engine
