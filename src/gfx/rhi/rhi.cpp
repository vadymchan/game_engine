#include "gfx/rhi/rhi.h"

namespace game_engine {

RHI* g_rhi = nullptr;

const RTClearValue RTClearValue::Invalid = RTClearValue();

int32_t GMaxCheckCountForRealTimeShaderUpdate = 10;
int32_t GSleepMSForRealTimeShaderUpdate       = 100;
bool    GUseRealTimeShaderUpdate              = true;

std::shared_ptr<Texture>  GWhiteTexture;
std::shared_ptr<Texture>  GBlackTexture;
std::shared_ptr<Texture>  GWhiteCubeTexture;
std::shared_ptr<Texture>  GNormalTexture;
std::shared_ptr<Material> GDefaultMaterial = nullptr;

TResourcePool<Shader, MutexRWLock> RHI::ShaderPool;

bool RHI::init(const std::shared_ptr<Window>& window) {
  return false;
}

void RHI::OnInitRHI() {
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

  GDefaultMaterial = std::make_shared<Material>();
  GDefaultMaterial->TexData[(int32_t)Material::EMaterialTextureType::Albedo]
      .m_texture
      = GWhiteTexture.get();
  GDefaultMaterial->TexData[(int32_t)Material::EMaterialTextureType::Normal]
      .m_texture
      = GNormalTexture.get();
}

void RHI::release() {
  GWhiteTexture.reset();
  GBlackTexture.reset();
  GWhiteCubeTexture.reset();
  GNormalTexture.reset();
  GDefaultMaterial.reset();

  Shader::ReleaseCheckUpdateShaderThread();
  ShaderPool.Release();
}

RHI::RHI() {
}

}  // namespace game_engine