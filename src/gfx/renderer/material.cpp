
#include "gfx/renderer/material.h"

#include "gfx/rhi/rhi.h"

namespace game_engine {

jTexture* jMaterial::GetTexture(EMaterialTextureType InType) const {
  assert(EMaterialTextureType::Albedo <= InType);
  assert(EMaterialTextureType::Max > InType);

  if (!TexData[(int32_t)InType].Texture) {
    if (InType == EMaterialTextureType::Normal) {
      return GNormalTexture.get();
    }
    return GBlackTexture.get();
  }

  return TexData[(int32_t)InType].Texture;
}

const std::shared_ptr<jShaderBindingInstance>&
    jMaterial::CreateShaderBindingInstance() {
  if (NeedToUpdateShaderBindingInstance) {
    NeedToUpdateShaderBindingInstance = false;

    int32_t                              BindingPoint = 0;
    jShaderBindingArray                   ShaderBindingArray;
    jShaderBindingResourceInlineAllocator ResourceInlineAllactor;

    for (int32_t i = 0; i < (int32_t)EMaterialTextureType::Max; ++i) {
      const TextureData& TextureData = TexData[i];
      const jTexture*   Texture     = TextureData.GetTexture();

      if (!Texture) {
        if ((int32_t)EMaterialTextureType::Normal == i) {
          Texture = GNormalTexture.get();
        } else if ((int32_t)EMaterialTextureType::Env == i) {
          Texture = GWhiteCubeTexture.get();
        } else {
          Texture = GWhiteTexture.get();
        }
      }

      ShaderBindingArray.Add(jShaderBinding(
          BindingPoint++,
          1,
          EShaderBindingType::TEXTURE_SAMPLER_SRV,
          false,
          EShaderAccessStageFlag::ALL_GRAPHICS,
          ResourceInlineAllactor.Alloc<jTextureResource>(Texture, nullptr)));
    }

    if (ShaderBindingInstance) {
      ShaderBindingInstance->Free();
    }

    ShaderBindingInstance = g_rhi->CreateShaderBindingInstance(
        ShaderBindingArray, jShaderBindingInstanceType::MultiFrame);
  }
  return ShaderBindingInstance;
}

}  // namespace game_engine
