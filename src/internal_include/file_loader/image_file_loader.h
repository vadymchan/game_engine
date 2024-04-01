#ifndef GAME_ENGINE_IMAGE_FILE_LOADER_H
#define GAME_ENGINE_IMAGE_FILE_LOADER_H

#include "gfx/rhi/name.h"
#include "gfx/rhi/rhi_type.h"

#include <vulkan/vulkan.h>

namespace game_engine {

struct ImageSubResourceData {
  int32_t  Format   = 0;
  uint32_t Width    = 0;
  uint32_t Height   = 0;
  uint32_t MipLevel = 0;
  uint32_t Depth    = 0;
  uint32_t RowPitch = 0;
  uint64_t Offset   = 0;
};

struct ImageBulkData {
  std::vector<unsigned char>        ImageData;
  std::vector<ImageSubResourceData> SubresourceFootprints;
};

struct ImageData {
  bool           CreateMipmapIfPossible = true;
  bool           sRGB                   = false;
  int32_t        Width                  = 0;
  int32_t        Height                 = 0;
  int32_t        MipLevel               = 1;
  int32_t        LayerCount             = 1;
  Name           Filename;
  ETextureFormat Format      = ETextureFormat::RGBA8;
  EFormatType    FormatType  = EFormatType::UNSIGNED_BYTE;
  ETextureType   TextureType = ETextureType::TEXTURE_2D;
  ImageBulkData  imageBulkData;  // TODO: ImageBulkData rhi_vk
};
}  // namespace game_engine

#endif  // GAME_ENGINE_IMAGE_FILE_LOADER_H