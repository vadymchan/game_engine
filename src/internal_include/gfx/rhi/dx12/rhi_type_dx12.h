#ifndef GAME_ENGINE_RHI_TYPE_DX12_H
#define GAME_ENGINE_RHI_TYPE_DX12_H

#include "platform/windows/windows_platform_setup.h"

#ifdef GAME_ENGINE_RHI_DX12

#include "gfx/rhi/rhi_type.h"

namespace game_engine {

// clang-format off

DXGI_FORMAT g_getDX12TextureFormat(ETextureFormat textureFormat);
ETextureFormat g_getDX12TextureFormat(DXGI_FORMAT formatType);

inline auto g_getDX12TextureComponentCount(ETextureFormat type) {
    static const std::unordered_map<ETextureFormat, int> textureComponentCountMapping = {
        {ETextureFormat::RGB8,       4},  // not support rgb8 -> rgba8
        {ETextureFormat::RGB32F,     4},  // not support rgb32 -> rgba32
        {ETextureFormat::RGB16F,     4},  // not support rgb16 -> rgba16
        {ETextureFormat::R11G11B10F, 3},

        {ETextureFormat::RGBA8,      4},
        {ETextureFormat::RGBA16F,    4},
        {ETextureFormat::RGBA32F,    4},
        {ETextureFormat::RGBA8SI,    4},
        {ETextureFormat::RGBA8UI,    4},

        {ETextureFormat::BGRA8,      4},

        {ETextureFormat::R8,         1},
        {ETextureFormat::R16F,       1},
        {ETextureFormat::R32F,       1},
        {ETextureFormat::R8UI,       1},
        {ETextureFormat::R32UI,      1},

        {ETextureFormat::RG8,        2},
        {ETextureFormat::RG16F,      2},
        {ETextureFormat::RG32F,      2},

        {ETextureFormat::D16,        1},
        {ETextureFormat::D16_S8,     2},
        {ETextureFormat::D24,        1},
        {ETextureFormat::D24_S8,     2},
        {ETextureFormat::D32,        1},
        {ETextureFormat::D32_S8,     2}
    };

    return getEnumMapping(textureComponentCountMapping, type, 0);
}

inline auto g_getDX12TexturePixelSize(ETextureFormat type) {
      static const std::unordered_map<ETextureFormat, int> textureFormatSizeMapping = {
        {ETextureFormat::RGB8,       4},  // not support rgb8 -> rgba8
        {ETextureFormat::RGB32F,     16}, // not support rgb32 -> rgba32
        {ETextureFormat::RGB16F,     8},  // not support rgb16 -> rgba16
        {ETextureFormat::R11G11B10F, 4},

        {ETextureFormat::RGBA8,      4},
        {ETextureFormat::RGBA16F,    8},
        {ETextureFormat::RGBA32F,    16},
        {ETextureFormat::RGBA8SI,    4},
        {ETextureFormat::RGBA8UI,    4},

        {ETextureFormat::BGRA8,      4},

        {ETextureFormat::R8,         1},
        {ETextureFormat::R16F,       2},
        {ETextureFormat::R32F,       4},
        {ETextureFormat::R8UI,       1},
        {ETextureFormat::R32UI,      4},

        {ETextureFormat::RG8,        2},
        {ETextureFormat::RG16F,      2},
        {ETextureFormat::RG32F,      4},

        {ETextureFormat::D16,        2},
        {ETextureFormat::D16_S8,     3},
        {ETextureFormat::D24,        3},
        {ETextureFormat::D24_S8,     4},
        {ETextureFormat::D32,        4},
        {ETextureFormat::D32_S8,     5}
    };

    return getEnumMapping(textureFormatSizeMapping, type, 0);
}

inline ETextureType g_getDX12TextureDemension(D3D12_RESOURCE_DIMENSION type,
                                              bool isArray) {
  switch (type) {
    case D3D12_RESOURCE_DIMENSION_TEXTURE2D: {
      return isArray ? ETextureType::TEXTURE_2D_ARRAY
                     : ETextureType::TEXTURE_2D;
    }
    case D3D12_RESOURCE_DIMENSION_TEXTURE3D: {
      return isArray ? ETextureType::TEXTURE_3D_ARRAY
                     : ETextureType::TEXTURE_3D;
    }
    case D3D12_RESOURCE_DIMENSION_UNKNOWN:
    case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
    case D3D12_RESOURCE_DIMENSION_BUFFER:
    default:
      return ETextureType::TEXTURE_2D;
  }
}

inline D3D12_RESOURCE_DIMENSION g_getDX12TextureDemension(ETextureType type) {
  switch (type) {
    case ETextureType::TEXTURE_1D:
      return D3D12_RESOURCE_DIMENSION_TEXTURE1D;
    case ETextureType::TEXTURE_2D:
    case ETextureType::TEXTURE_2D_ARRAY:
    case ETextureType::TEXTURE_CUBE:
      return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    case ETextureType::TEXTURE_3D:
    case ETextureType::TEXTURE_3D_ARRAY:
      return D3D12_RESOURCE_DIMENSION_TEXTURE3D;
  }
  return D3D12_RESOURCE_DIMENSION_UNKNOWN;
}

D3D12_DESCRIPTOR_HEAP_TYPE g_getDX12DescriptorHeapType(EDescriptorHeapTypeDX12 heapType);
EDescriptorHeapTypeDX12 g_getDX12DescriptorHeapType(D3D12_DESCRIPTOR_HEAP_TYPE heapType);

D3D12_DESCRIPTOR_RANGE_TYPE g_getDX12ShaderBindingType(EShaderBindingType bindingType);
EShaderBindingType g_getDX12ShaderBindingType(D3D12_DESCRIPTOR_RANGE_TYPE bindingType);

D3D12_TEXTURE_ADDRESS_MODE g_getDX12TextureAddressMode(ETextureAddressMode addressMode);
ETextureAddressMode g_getDX12TextureAddressMode(D3D12_TEXTURE_ADDRESS_MODE addressMode);

D3D12_COMPARISON_FUNC g_getDX12CompareOp(ECompareOp compareOp);
ECompareOp g_getDX12CompareOp(D3D12_COMPARISON_FUNC compareOp);

D3D12_STENCIL_OP g_getDX12StencilOp(EStencilOp stencilOp);
EStencilOp g_getDX12StencilOp(D3D12_STENCIL_OP stencilOp);

D3D12_PRIMITIVE_TOPOLOGY_TYPE g_getDX12PrimitiveTopologyTypeOnly(EPrimitiveType type);
EPrimitiveType g_getDX12PrimitiveTopologyTypeOnly(D3D12_PRIMITIVE_TOPOLOGY_TYPE type);

D3D_PRIMITIVE_TOPOLOGY g_getDX12PrimitiveTopology(EPrimitiveType type);
EPrimitiveType g_getDX12PrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY type);

D3D12_INPUT_CLASSIFICATION g_getDX12VertexInputRate(EVertexInputRate inputRate);
EVertexInputRate g_getDX12VertexInputRate(D3D12_INPUT_CLASSIFICATION inputRate);

D3D12_FILL_MODE g_getDX12PolygonMode(EPolygonMode polygonMode);
EPolygonMode g_getDX12PolygonMode(D3D12_FILL_MODE polygonMode);

D3D12_CULL_MODE g_getDX12CullMode(ECullMode cullMode);
ECullMode g_getDX12CullMode(D3D12_CULL_MODE cullMode);

D3D12_BLEND g_getDX12BlendFactor(EBlendFactor type);
EBlendFactor g_getDX12BlendFactor(D3D12_BLEND type);

D3D12_BLEND_OP g_getDX12BlendOp(EBlendOp blendOp);
EBlendOp g_getDX12BlendOp(D3D12_BLEND_OP blendOp);



inline uint8_t g_getDX12ColorMask(EColorMask type) {
  uint8_t result = 0;

  if (EColorMask::NONE == type) {
    return result;
  }

  if (EColorMask::ALL == type) {
    result = D3D12_COLOR_WRITE_ENABLE_RED | D3D12_COLOR_WRITE_ENABLE_GREEN
           | D3D12_COLOR_WRITE_ENABLE_BLUE | D3D12_COLOR_WRITE_ENABLE_ALPHA;
  } else {
    if (!!(EColorMask::R & type)) {
      result |= D3D12_COLOR_WRITE_ENABLE_RED;
    }
    if (!!(EColorMask::G & type)) {
      result |= D3D12_COLOR_WRITE_ENABLE_GREEN;
    }
    if (!!(EColorMask::B & type)) {
      result |= D3D12_COLOR_WRITE_ENABLE_BLUE;
    }
    if (!!(EColorMask::A & type)) {
      result |= D3D12_COLOR_WRITE_ENABLE_ALPHA;
    }
  }
  return result;
}

inline D3D12_FILTER g_getDX12TextureFilter(ETextureFilter minification,
                                           ETextureFilter magnification,
                                           bool isComparison = false) {
  // Comparison is used for ShadowMap
  if (isComparison) {
    D3D12_FILTER filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
    switch (minification) {
      case ETextureFilter::NEAREST:
      case ETextureFilter::NEAREST_MIPMAP_NEAREST:
        if (magnification == ETextureFilter::NEAREST) {
          filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
        } else if (magnification == ETextureFilter::LINEAR) {
          filter = D3D12_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT;
        } else if (magnification == ETextureFilter::NEAREST_MIPMAP_NEAREST) {
          filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
        } else if (magnification == ETextureFilter::LINEAR_MIPMAP_NEAREST) {
          filter = D3D12_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT;
        } else if (magnification == ETextureFilter::NEAREST_MIPMAP_LINEAR) {
          filter = D3D12_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR;
        } else if (magnification == ETextureFilter::LINEAR_MIPMAP_LINEAR) {
          filter = D3D12_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR;
        }
        break;
      case ETextureFilter::LINEAR:
      case ETextureFilter::LINEAR_MIPMAP_NEAREST:
        if (magnification == ETextureFilter::NEAREST) {
          filter = D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT;
        } else if (magnification == ETextureFilter::LINEAR) {
          filter = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
        } else if (magnification == ETextureFilter::NEAREST_MIPMAP_NEAREST) {
          filter = D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT;
        } else if (magnification == ETextureFilter::LINEAR_MIPMAP_NEAREST) {
          filter = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
        } else if (magnification == ETextureFilter::NEAREST_MIPMAP_LINEAR) {
          filter = D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
        } else if (magnification == ETextureFilter::LINEAR_MIPMAP_LINEAR) {
          filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
        }
        break;
      case ETextureFilter::NEAREST_MIPMAP_LINEAR:
        if (magnification == ETextureFilter::NEAREST) {
          filter = D3D12_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR;
        } else if (magnification == ETextureFilter::LINEAR) {
          filter = D3D12_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR;
        } else if (magnification == ETextureFilter::NEAREST_MIPMAP_NEAREST) {
          // If mipmaps overlap, upload them as linear and process them. Needs
          // organization.
          filter = D3D12_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR;
        } else if (magnification == ETextureFilter::LINEAR_MIPMAP_NEAREST) {
          // If mipmaps overlap, upload them as linear and process them. Needs
          // organization.
          filter = D3D12_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR;
        } else if (magnification == ETextureFilter::NEAREST_MIPMAP_LINEAR) {
          filter = D3D12_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR;
        } else if (magnification == ETextureFilter::LINEAR_MIPMAP_LINEAR) {
          filter = D3D12_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR;
        }
        break;
      case ETextureFilter::LINEAR_MIPMAP_LINEAR:
        if (magnification == ETextureFilter::NEAREST) {
          filter = D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
        } else if (magnification == ETextureFilter::LINEAR) {
          filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
        } else if (magnification == ETextureFilter::NEAREST_MIPMAP_NEAREST) {
          // If mipmaps overlap, upload them as linear and process them. Needs
          // organization.
          filter = D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
        } else if (magnification == ETextureFilter::LINEAR_MIPMAP_NEAREST) {
          // If mipmaps overlap, upload them as linear and process them. Needs
          // organization.
          filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
        } else if (magnification == ETextureFilter::NEAREST_MIPMAP_LINEAR) {
          filter = D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
        } else if (magnification == ETextureFilter::LINEAR_MIPMAP_LINEAR) {
          filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
        }
        break;
      default:
        break;
    }
    return filter;
  } else {
    D3D12_FILTER filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
    switch (minification) {
      case ETextureFilter::NEAREST:
      case ETextureFilter::NEAREST_MIPMAP_NEAREST:
        if (magnification == ETextureFilter::NEAREST) {
          filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
        } else if (magnification == ETextureFilter::LINEAR) {
          filter = D3D12_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
        } else if (magnification == ETextureFilter::NEAREST_MIPMAP_NEAREST) {
          filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
        } else if (magnification == ETextureFilter::LINEAR_MIPMAP_NEAREST) {
          filter = D3D12_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
        } else if (magnification == ETextureFilter::NEAREST_MIPMAP_LINEAR) {
          filter = D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR;
        } else if (magnification == ETextureFilter::LINEAR_MIPMAP_LINEAR) {
          filter = D3D12_FILTER_MIN_POINT_MAG_MIP_LINEAR;
        }
        break;
      case ETextureFilter::LINEAR:
      case ETextureFilter::LINEAR_MIPMAP_NEAREST:
        if (magnification == ETextureFilter::NEAREST) {
          filter = D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT;
        } else if (magnification == ETextureFilter::LINEAR) {
          filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
        } else if (magnification == ETextureFilter::NEAREST_MIPMAP_NEAREST) {
          filter = D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT;
        } else if (magnification == ETextureFilter::LINEAR_MIPMAP_NEAREST) {
          filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
        } else if (magnification == ETextureFilter::NEAREST_MIPMAP_LINEAR) {
          filter = D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
        } else if (magnification == ETextureFilter::LINEAR_MIPMAP_LINEAR) {
          filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
        }
        break;
      case ETextureFilter::NEAREST_MIPMAP_LINEAR:
        if (magnification == ETextureFilter::NEAREST) {
          filter = D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR;
        } else if (magnification == ETextureFilter::LINEAR) {
          filter = D3D12_FILTER_MIN_POINT_MAG_MIP_LINEAR;
        } else if (magnification == ETextureFilter::NEAREST_MIPMAP_NEAREST) {
          // If mipmaps overlap, upload them as linear and process them. Needs
          // organization.
          filter = D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR;
        } else if (magnification == ETextureFilter::LINEAR_MIPMAP_NEAREST) {
          // If mipmaps overlap, upload them as linear and process them. Needs
          // organization.
          filter = D3D12_FILTER_MIN_POINT_MAG_MIP_LINEAR;
        } else if (magnification == ETextureFilter::NEAREST_MIPMAP_LINEAR) {
          filter = D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR;
        } else if (magnification == ETextureFilter::LINEAR_MIPMAP_LINEAR) {
          filter = D3D12_FILTER_MIN_POINT_MAG_MIP_LINEAR;
        }
        break;
      case ETextureFilter::LINEAR_MIPMAP_LINEAR:
        if (magnification == ETextureFilter::NEAREST) {
          filter = D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
        } else if (magnification == ETextureFilter::LINEAR) {
          filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
        } else if (magnification == ETextureFilter::NEAREST_MIPMAP_NEAREST) {
          // If mipmaps overlap, upload them as linear and process them. Needs
          // organization.
          filter = D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
        } else if (magnification == ETextureFilter::LINEAR_MIPMAP_NEAREST) {
          // If mipmaps overlap, upload them as linear and process them. Needs
          // organization.
          filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
        } else if (magnification == ETextureFilter::NEAREST_MIPMAP_LINEAR) {
          filter = D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
        } else if (magnification == ETextureFilter::LINEAR_MIPMAP_LINEAR) {
          filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
        }
        break;
      default:
        break;
    }
    return filter;
  }
}

inline void g_getDepthFormatForSRV(DXGI_FORMAT& texFormat,
                                   DXGI_FORMAT& srvFormat,
                                   DXGI_FORMAT  originalTexFormat) {
  switch (originalTexFormat) {
    case DXGI_FORMAT_D16_UNORM:
      texFormat = DXGI_FORMAT_R16_TYPELESS;
      srvFormat = DXGI_FORMAT_R16_UNORM;
      break;
    case DXGI_FORMAT_D24_UNORM_S8_UINT:
      texFormat = DXGI_FORMAT_R24G8_TYPELESS;
      srvFormat = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
      break;
    case DXGI_FORMAT_D32_FLOAT:
      texFormat = DXGI_FORMAT_R32_TYPELESS;
      srvFormat = DXGI_FORMAT_R32_FLOAT;
      break;
    case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
      texFormat = DXGI_FORMAT_R32G8X24_TYPELESS;
      srvFormat = DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
      break;
    default:
      assert(0);
      break;
  }
}

D3D12_RESOURCE_STATES g_getDX12ResourceLayout(EResourceLayout resourceLayout);
EResourceLayout g_getDX12ResourceLayout(D3D12_RESOURCE_STATES resourceState);


// Generated from CreateResource, CreateUploadResource
struct CreatedResource : public std::enable_shared_from_this<CreatedResource> {
  // ======= BEGIN: public nested types =======================================

  enum class EType : uint8_t {
    Standalone,    // CommittedResource
    ResourcePool,  // PlacedResource
    Swapchain,     // no need s_release by me
  };

  // ======= END: public nested types   =======================================

  // ======= BEGIN: public static methods =====================================

  static std::shared_ptr<CreatedResource> s_createdFromStandalone(
      const ComPtr<ID3D12Resource>& resource) {
    return std::shared_ptr<CreatedResource>(
        new CreatedResource(EType::Standalone, resource));
  }

  static std::shared_ptr<CreatedResource> s_createdFromResourcePool(
      const ComPtr<ID3D12Resource>& resource) {
    return std::shared_ptr<CreatedResource>(
        new CreatedResource(EType::ResourcePool, resource));
  }

  static std::shared_ptr<CreatedResource> s_createdFromSwapchain(
      const ComPtr<ID3D12Resource>& resource) {
    return std::shared_ptr<CreatedResource>(
        new CreatedResource(EType::Swapchain, resource));
  }

  // ======= END: public static methods   =====================================

  // ======= BEGIN: public destructor =========================================

  ~CreatedResource() { free(); }

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public getters ============================================

  ID3D12Resource* get() const {
    return m_resource_ ? (*m_resource_).Get() : nullptr;
  }

  // TODO: not used
  const ComPtr<ID3D12Resource>& getPtr() const { return *m_resource_; }

  uint64_t getGPUVirtualAddress() const {
    return m_resource_ ? (*m_resource_)->GetGPUVirtualAddress() : 0;
  }

  // ======= END: public getters   ============================================

  // ======= BEGIN: public misc methods =======================================

  bool isValid() const { return m_resource_ && (*m_resource_).Get(); }

  void free();

  // ======= END: public misc methods   =======================================

  // ======= BEGIN: public misc fields ========================================

  EType                                   m_resourceType_ = EType::Standalone;
  // TODO: consider remove shared ptr and use pure Resource
  std::shared_ptr<ComPtr<ID3D12Resource>> m_resource_;

  // ======= END: public misc fields   ========================================

  private:
  // ======= BEGIN: private constructors ======================================

  CreatedResource() {}

  CreatedResource(EType type, const ComPtr<ID3D12Resource>& resource)
      : m_resourceType_(type)
      , m_resource_(std::make_shared<ComPtr<ID3D12Resource>>(resource)) {}

  // prevent copying
  CreatedResource(const CreatedResource&) = delete;

  // ======= END: private constructors   ======================================

  // ======= BEGIN: private overloaded operators ==============================

  CreatedResource& operator=(const CreatedResource&) = delete;

  // ======= END: private overloaded operators   ==============================
};

}  // namespace game_engine

#endif  // GAME_ENGINE_RHI_DX12

#endif  // GAME_ENGINE_RHI_TYPE_DX12_H
