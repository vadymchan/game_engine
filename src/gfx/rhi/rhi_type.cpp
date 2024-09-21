#include "gfx/rhi/rhi_type.h"

namespace game_engine {

// clang-format off

// ======= BEGIN: Texture format to format type mapping =======================

    static const std::unordered_map<ETextureFormat, EFormatType> formatMappingToFormatType = {
  { ETextureFormat::RGB8,       EFormatType::UNSIGNED_BYTE },
  { ETextureFormat::RGB32F,     EFormatType::HALF          },
  { ETextureFormat::RGB16F,     EFormatType::HALF          },
  { ETextureFormat::R11G11B10F, EFormatType::HALF          },
    
  { ETextureFormat::RGBA8,      EFormatType::UNSIGNED_BYTE },
  { ETextureFormat::RGBA16F,    EFormatType::HALF          },
  { ETextureFormat::RGBA32F,    EFormatType::FLOAT         },
  { ETextureFormat::RGBA8SI,    EFormatType::INT           },
  { ETextureFormat::RGBA8UI,    EFormatType::INT           },
    
  { ETextureFormat::BGRA8,      EFormatType::UNSIGNED_BYTE },
    
  { ETextureFormat::R8,         EFormatType::UNSIGNED_BYTE },
  { ETextureFormat::R16F,       EFormatType::HALF          },
  { ETextureFormat::R32F,       EFormatType::FLOAT         },
  { ETextureFormat::R8UI,       EFormatType::INT           },
  { ETextureFormat::R32UI,      EFormatType::INT           },
  { ETextureFormat::RG8,        EFormatType::UNSIGNED_BYTE },
  { ETextureFormat::RG16F,      EFormatType::HALF          },
  { ETextureFormat::RG32F,      EFormatType::FLOAT         },
    
  { ETextureFormat::D16,        EFormatType::SHORT_INT     },
  { ETextureFormat::D16_S8,     EFormatType::INT           },
  { ETextureFormat::D24,        EFormatType::INT           },
  { ETextureFormat::D24_S8,     EFormatType::INT           },
  { ETextureFormat::D32,        EFormatType::FLOAT         },
  { ETextureFormat::D32_S8,     EFormatType::FLOAT         },
    
  { ETextureFormat::BC1_UNORM,  EFormatType::UNSIGNED_BYTE },
  { ETextureFormat::BC2_UNORM,  EFormatType::UNSIGNED_BYTE },
  { ETextureFormat::BC3_UNORM,  EFormatType::UNSIGNED_BYTE },
  { ETextureFormat::BC4_UNORM,  EFormatType::UNSIGNED_BYTE },
  { ETextureFormat::BC4_SNORM,  EFormatType::UNSIGNED_BYTE },
  { ETextureFormat::BC5_UNORM,  EFormatType::UNSIGNED_BYTE },
  { ETextureFormat::BC5_SNORM,  EFormatType::UNSIGNED_BYTE },
  { ETextureFormat::BC6H_UF16,  EFormatType::HALF          },
  { ETextureFormat::BC6H_SF16,  EFormatType::HALF          },
  { ETextureFormat::BC7_UNORM,  EFormatType::HALF          }
};

// ======= END: Texture format to format type mapping =========================

// ======= BEGIN: Format type to texture format reverse mapping & functions ===

static const std::unordered_map<EFormatType, ETextureFormat> formatMappingToTextureFormat
    = reverseMap(formatMappingToFormatType);

EFormatType getTextureFormat(ETextureFormat textureFormat) {
  return getEnumMapping(formatMappingToFormatType, textureFormat, EFormatType::MAX);
}

ETextureFormat getTextureFormat(EFormatType formatType) {
  return getEnumMapping(formatMappingToTextureFormat, formatType, ETextureFormat::MAX);
}

// ======= END: Format type to texture format reverse mapping & functions =====

// clang-format on

}  // namespace game_engine
