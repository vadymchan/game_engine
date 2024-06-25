#include "gfx/rhi/rhi.h"

namespace game_engine {

jRHI* g_rhi = nullptr;

const jRTClearValue jRTClearValue::Invalid = jRTClearValue();

int32_t GMaxCheckCountForRealTimeShaderUpdate = 10;
int32_t GSleepMSForRealTimeShaderUpdate       = 100;
bool    GUseRealTimeShaderUpdate              = true;

std::shared_ptr<jTexture>  GWhiteTexture;
std::shared_ptr<jTexture>  GBlackTexture;
std::shared_ptr<jTexture>  GWhiteCubeTexture;
std::shared_ptr<jTexture>  GNormalTexture;
std::shared_ptr<jMaterial> GDefaultMaterial = nullptr;

TResourcePool<Shader, MutexRWLock> jRHI::ShaderPool;

bool jRHI::init(const std::shared_ptr<Window>& window) {
  return false;
}

void jRHI::OnInitRHI() {
  ImageData image;
  image.imageBulkData.ImageData = {255, 255, 255, 255};
  image.Width                   = 1;
  image.Height                  = 1;
  image.Format                  = ETextureFormat::RGBA8;
  image.FormatType              = EFormatType::UNSIGNED_BYTE;
  image.sRGB                    = false;

  image.imageBulkData.ImageData = {255, 255, 255, 255};
  GWhiteTexture                 = CreateTextureFromData(&image);

  image.imageBulkData.ImageData = {0, 0, 0, 255};
  GBlackTexture                 = CreateTextureFromData(&image);

  image.imageBulkData.ImageData = {255 / 2, 255 / 2, 255, 0};
  GNormalTexture                = CreateTextureFromData(&image);

  image.TextureType             = ETextureType::TEXTURE_CUBE;
  image.LayerCount              = 6;
  image.imageBulkData.ImageData = {255, 255, 255, 200, 255, 255};
  GWhiteCubeTexture             = CreateTextureFromData(&image);

  GDefaultMaterial = std::make_shared<jMaterial>();
  GDefaultMaterial->TexData[(int32_t)jMaterial::EMaterialTextureType::Albedo]
      .Texture
      = GWhiteTexture.get();
  GDefaultMaterial->TexData[(int32_t)jMaterial::EMaterialTextureType::Normal]
      .Texture
      = GNormalTexture.get();
}

void jRHI::release() {
  GWhiteTexture.reset();
  GBlackTexture.reset();
  GWhiteCubeTexture.reset();
  GNormalTexture.reset();
  GDefaultMaterial.reset();

  Shader::ReleaseCheckUpdateShaderThread();
  ShaderPool.Release();
}

jRHI::jRHI() {
}

}  // namespace game_engine