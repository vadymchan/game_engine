
#include "gfx/renderer/material.h"

#include "gfx/rhi/rhi.h"

namespace game_engine {

Texture* Material::GetTexture(EMaterialTextureType InType) const {
  assert(EMaterialTextureType::Albedo <= InType);
  assert(EMaterialTextureType::Max > InType);

  if (!TexData[(int32_t)InType].m_texture) {
    if (InType == EMaterialTextureType::Normal) {
      return GNormalTexture.get();
    }
    return GBlackTexture.get();
  }

  return TexData[(int32_t)InType].m_texture;
}

const std::shared_ptr<ShaderBindingInstance>&
    Material::CreateShaderBindingInstance() {
  if (NeedToUpdateShaderBindingInstance) {
    NeedToUpdateShaderBindingInstance = false;

    int32_t                              BindingPoint = 0;
    ShaderBindingArray                   shaderBindingArray;
    ShaderBindingResourceInlineAllocator ResourceInlineAllactor;

    for (int32_t i = 0; i < (int32_t)EMaterialTextureType::Max; ++i) {
      const TextureData& TextureData = TexData[i];
      const Texture*   texture     = TextureData.GetTexture();

      if (!texture) {
        if ((int32_t)EMaterialTextureType::Normal == i) {
          texture = GNormalTexture.get();
        } else if ((int32_t)EMaterialTextureType::Env == i) {
          texture = GWhiteCubeTexture.get();
        } else {
          texture = GWhiteTexture.get();
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

    if (m_shaderBindingInstance) {
      m_shaderBindingInstance->Free();
    }

    m_shaderBindingInstance = g_rhi->CreateShaderBindingInstance(
        shaderBindingArray, ShaderBindingInstanceType::MultiFrame);
  }
  return m_shaderBindingInstance;
}

}  // namespace game_engine
