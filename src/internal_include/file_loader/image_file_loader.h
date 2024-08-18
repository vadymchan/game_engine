#ifndef GAME_ENGINE_IMAGE_FILE_LOADER_H
#define GAME_ENGINE_IMAGE_FILE_LOADER_H

#include "gfx/rhi/name.h"
#include "gfx/rhi/rhi_type.h"

#include <vulkan/vulkan.h>

namespace game_engine {

struct ImageSubResourceData {
  int32_t  m_format_   = 0;
  uint32_t m_width_    = 0;
  uint32_t m_height_   = 0;
  uint32_t m_mipLevel_ = 0;
  uint32_t m_depth_    = 0;
  uint32_t m_rowPitch_ = 0;
  uint64_t m_offset_   = 0;
};

struct ImageBulkData {
  std::vector<unsigned char>        m_imageData_;
  std::vector<ImageSubResourceData> m_subresourceFootprints_;
};

struct ImageData {
  bool           m_createMipmapIfPossible_ = true;
  bool           m_sRGB_                   = false;
  int32_t        m_width_                  = 0;
  int32_t        m_height_                 = 0;
  int32_t        m_mipLevel_               = 1;
  int32_t        m_layerCount_             = 1;
  // TODO: currently not used
  Name           m_filename_;
  ETextureFormat m_format_      = ETextureFormat::RGBA8;
  EFormatType    m_formatType_  = EFormatType::UNSIGNED_BYTE;
  ETextureType   m_textureType_ = ETextureType::TEXTURE_2D;
  ImageBulkData  m_imageBulkData_;  // TODO: ImageBulkData rhi_vk
};
}  // namespace game_engine

#endif  // GAME_ENGINE_IMAGE_FILE_LOADER_H