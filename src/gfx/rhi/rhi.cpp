#include "gfx/rhi/rhi.h"

namespace game_engine {

RHI* g_rhi = nullptr;


int32_t g_maxCheckCountForRealTimeShaderUpdate = 10;
int32_t g_sleepMSForRealTimeShaderUpdate       = 100;
bool    g_useRealTimeShaderUpdate              = true;

std::shared_ptr<Texture>  g_whiteTexture;
std::shared_ptr<Texture>  g_blackTexture;
std::shared_ptr<Texture>  g_whiteCubeTexture;
std::shared_ptr<Texture>  g_normalTexture;
std::shared_ptr<Material> g_defaultMaterial = nullptr;

TResourcePool<Shader, MutexRWLock> RHI::s_shaderPool;

const RTClearValue RTClearValue::s_kInvalid = RTClearValue();

bool RHI::init(const std::shared_ptr<Window>& window) {
  return false;
}

void RHI::OnInitRHI() {
  ImageData image;
  image.m_imageBulkData_.m_imageData_ = {255, 255, 255, 255};
  image.m_width_                   = 1;
  image.m_height_                  = 1;
  image.m_format_                  = ETextureFormat::RGBA8;
  image.m_formatType_              = EFormatType::UNSIGNED_BYTE;
  image.m_sRGB_                    = false;

  image.m_imageBulkData_.m_imageData_ = {255, 255, 255, 255};
  g_whiteTexture                 = CreateTextureFromData(&image);

  image.m_imageBulkData_.m_imageData_ = {0, 0, 0, 255};
  g_blackTexture                 = CreateTextureFromData(&image);

  image.m_imageBulkData_.m_imageData_ = {255 / 2, 255 / 2, 255, 0};
  g_normalTexture                = CreateTextureFromData(&image);

  image.m_textureType_             = ETextureType::TEXTURE_CUBE;
  image.m_layerCount_              = 6;
  image.m_imageBulkData_.m_imageData_ = {255, 255, 255, 200, 255, 255};
  g_whiteCubeTexture             = CreateTextureFromData(&image);

  g_defaultMaterial = std::make_shared<Material>();
  g_defaultMaterial->m_texData_[(int32_t)Material::EMaterialTextureType::Albedo]
      .m_texture_
      = g_whiteTexture.get();
  g_defaultMaterial->m_texData_[(int32_t)Material::EMaterialTextureType::Normal]
      .m_texture_
      = g_normalTexture.get();
}

void RHI::release() {
  g_whiteTexture.reset();
  g_blackTexture.reset();
  g_whiteCubeTexture.reset();
  g_normalTexture.reset();
  g_defaultMaterial.reset();

  Shader::ReleaseCheckUpdateShaderThread();
  s_shaderPool.Release();
}

RHI::RHI() {
}

}  // namespace game_engine