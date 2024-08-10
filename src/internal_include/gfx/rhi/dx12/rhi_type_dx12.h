#ifndef GAME_ENGINE_RHI_TYPE_DX12_H
#define GAME_ENGINE_RHI_TYPE_DX12_H

#include "gfx/rhi/rhi_type.h"
#include "platform/windows/windows_platform_setup.h"

namespace game_engine {

// clang-format off

GENERATE_CONVERSION_FUNCTION(GetDX12TextureFormat,
  CONVERSION_TYPE_ELEMENT(ETextureFormat::RGB8, DXGI_FORMAT_R8G8B8A8_UNORM),        // not support rgb8 -> rgba8
  CONVERSION_TYPE_ELEMENT(ETextureFormat::RGB32F, DXGI_FORMAT_R32G32B32A32_FLOAT),  // not support for UAV rgb32 -> rgba32, check this https://learn.microsoft.com/en-us/windows/win32/direct3d12/typed-unordered-access-view-loads
  CONVERSION_TYPE_ELEMENT(ETextureFormat::RGB16F, DXGI_FORMAT_R16G16B16A16_FLOAT),  // not support rgb16 -> rgba16
  CONVERSION_TYPE_ELEMENT(ETextureFormat::R11G11B10F, DXGI_FORMAT_R11G11B10_FLOAT),
  CONVERSION_TYPE_ELEMENT(ETextureFormat::RGBA8, DXGI_FORMAT_R8G8B8A8_UNORM),
  CONVERSION_TYPE_ELEMENT(ETextureFormat::RGBA16F, DXGI_FORMAT_R16G16B16A16_FLOAT),
  CONVERSION_TYPE_ELEMENT(ETextureFormat::RGBA32F, DXGI_FORMAT_R32G32B32A32_FLOAT),
  CONVERSION_TYPE_ELEMENT(ETextureFormat::RGBA8SI, DXGI_FORMAT_R8G8B8A8_SINT),
  CONVERSION_TYPE_ELEMENT(ETextureFormat::RGBA8UI, DXGI_FORMAT_R8G8B8A8_UINT),
  CONVERSION_TYPE_ELEMENT(ETextureFormat::BGRA8, DXGI_FORMAT_B8G8R8A8_UNORM),
  CONVERSION_TYPE_ELEMENT(ETextureFormat::R8, DXGI_FORMAT_R8_UNORM),
  CONVERSION_TYPE_ELEMENT(ETextureFormat::R16F, DXGI_FORMAT_R16_FLOAT),
  CONVERSION_TYPE_ELEMENT(ETextureFormat::R32F, DXGI_FORMAT_R32_FLOAT),
  CONVERSION_TYPE_ELEMENT(ETextureFormat::R8UI, DXGI_FORMAT_R8_UINT),
  CONVERSION_TYPE_ELEMENT(ETextureFormat::R32UI, DXGI_FORMAT_R32_UINT),
  CONVERSION_TYPE_ELEMENT(ETextureFormat::RG8, DXGI_FORMAT_R8G8_UNORM),
  CONVERSION_TYPE_ELEMENT(ETextureFormat::RG16F, DXGI_FORMAT_R16G16_FLOAT),
  CONVERSION_TYPE_ELEMENT(ETextureFormat::RG32F, DXGI_FORMAT_R32G32_FLOAT),
  CONVERSION_TYPE_ELEMENT(ETextureFormat::D16, DXGI_FORMAT_D16_UNORM),
  CONVERSION_TYPE_ELEMENT(ETextureFormat::D16_S8, DXGI_FORMAT_D24_UNORM_S8_UINT),   // not support d16_s8 -> d24_s8
  CONVERSION_TYPE_ELEMENT(ETextureFormat::D24, DXGI_FORMAT_D24_UNORM_S8_UINT),
  CONVERSION_TYPE_ELEMENT(ETextureFormat::D24_S8, DXGI_FORMAT_D24_UNORM_S8_UINT),
  CONVERSION_TYPE_ELEMENT(ETextureFormat::D32, DXGI_FORMAT_D32_FLOAT),
  CONVERSION_TYPE_ELEMENT(ETextureFormat::D32_S8, DXGI_FORMAT_D32_FLOAT),
  CONVERSION_TYPE_ELEMENT(ETextureFormat::BC1_UNORM, DXGI_FORMAT_BC1_UNORM),
  CONVERSION_TYPE_ELEMENT(ETextureFormat::BC2_UNORM, DXGI_FORMAT_BC2_UNORM),
  CONVERSION_TYPE_ELEMENT(ETextureFormat::BC3_UNORM, DXGI_FORMAT_BC3_UNORM),
  CONVERSION_TYPE_ELEMENT(ETextureFormat::BC4_UNORM, DXGI_FORMAT_BC4_UNORM),
  CONVERSION_TYPE_ELEMENT(ETextureFormat::BC4_SNORM, DXGI_FORMAT_BC4_SNORM),
  CONVERSION_TYPE_ELEMENT(ETextureFormat::BC5_UNORM, DXGI_FORMAT_BC5_UNORM),
  CONVERSION_TYPE_ELEMENT(ETextureFormat::BC5_SNORM, DXGI_FORMAT_BC5_SNORM),
  CONVERSION_TYPE_ELEMENT(ETextureFormat::BC6H_UF16, DXGI_FORMAT_BC6H_UF16),
  CONVERSION_TYPE_ELEMENT(ETextureFormat::BC6H_SF16, DXGI_FORMAT_BC6H_SF16),
  CONVERSION_TYPE_ELEMENT(ETextureFormat::BC7_UNORM, DXGI_FORMAT_BC7_UNORM))

inline auto GetDX12TextureComponentCount(ETextureFormat type) {
  GENERATE_STATIC_CONVERSION_ARRAY(
    CONVERSION_TYPE_ELEMENT(ETextureFormat::RGB8, 4),   // not support rgb8 -> rgba8
    CONVERSION_TYPE_ELEMENT(ETextureFormat::RGB32F, 4), // not support rgb32 -> rgba32
    CONVERSION_TYPE_ELEMENT(ETextureFormat::RGB16F, 4), // not support rgb16 -> rgba16
    CONVERSION_TYPE_ELEMENT(ETextureFormat::R11G11B10F, 3),

    CONVERSION_TYPE_ELEMENT(ETextureFormat::RGBA8, 4),
    CONVERSION_TYPE_ELEMENT(ETextureFormat::RGBA16F, 4),
    CONVERSION_TYPE_ELEMENT(ETextureFormat::RGBA32F, 4),
    CONVERSION_TYPE_ELEMENT(ETextureFormat::RGBA8SI, 4),
    CONVERSION_TYPE_ELEMENT(ETextureFormat::RGBA8UI, 4),

    CONVERSION_TYPE_ELEMENT(ETextureFormat::BGRA8, 4),

    CONVERSION_TYPE_ELEMENT(ETextureFormat::R8, 1),
    CONVERSION_TYPE_ELEMENT(ETextureFormat::R16F, 1),
    CONVERSION_TYPE_ELEMENT(ETextureFormat::R32F, 1),
    CONVERSION_TYPE_ELEMENT(ETextureFormat::R8UI, 1),
    CONVERSION_TYPE_ELEMENT(ETextureFormat::R32UI, 1),

    CONVERSION_TYPE_ELEMENT(ETextureFormat::RG8, 2),
    CONVERSION_TYPE_ELEMENT(ETextureFormat::RG16F, 2),
    CONVERSION_TYPE_ELEMENT(ETextureFormat::RG32F, 2),

    CONVERSION_TYPE_ELEMENT(ETextureFormat::D16, 1),
    CONVERSION_TYPE_ELEMENT(ETextureFormat::D16_S8, 2),
    CONVERSION_TYPE_ELEMENT(ETextureFormat::D24, 1),
    CONVERSION_TYPE_ELEMENT(ETextureFormat::D24_S8, 2),
    CONVERSION_TYPE_ELEMENT(ETextureFormat::D32, 1),
    CONVERSION_TYPE_ELEMENT(ETextureFormat::D32_S8, 2));
}

inline auto GetDX12TexturePixelSize(ETextureFormat type) {
  GENERATE_STATIC_CONVERSION_ARRAY(
    CONVERSION_TYPE_ELEMENT(ETextureFormat::RGB8, 4),    // not support rgb8 -> rgba8
    CONVERSION_TYPE_ELEMENT(ETextureFormat::RGB32F, 16), // not support rgb32 -> rgba32
    CONVERSION_TYPE_ELEMENT(ETextureFormat::RGB16F, 8),  // not support rgb16 -> rgba16
    CONVERSION_TYPE_ELEMENT(ETextureFormat::R11G11B10F, 4),

    CONVERSION_TYPE_ELEMENT(ETextureFormat::RGBA8, 4),
    CONVERSION_TYPE_ELEMENT(ETextureFormat::RGBA16F, 8),
    CONVERSION_TYPE_ELEMENT(ETextureFormat::RGBA32F, 16),
    CONVERSION_TYPE_ELEMENT(ETextureFormat::RGBA8SI, 4),
    CONVERSION_TYPE_ELEMENT(ETextureFormat::RGBA8UI, 4),

    CONVERSION_TYPE_ELEMENT(ETextureFormat::BGRA8, 4),

    CONVERSION_TYPE_ELEMENT(ETextureFormat::R8, 1),
    CONVERSION_TYPE_ELEMENT(ETextureFormat::R16F, 2),
    CONVERSION_TYPE_ELEMENT(ETextureFormat::R32F, 4),
    CONVERSION_TYPE_ELEMENT(ETextureFormat::R8UI, 1),
    CONVERSION_TYPE_ELEMENT(ETextureFormat::R32UI, 4),

    CONVERSION_TYPE_ELEMENT(ETextureFormat::RG8, 2),
    CONVERSION_TYPE_ELEMENT(ETextureFormat::RG16F, 2),
    CONVERSION_TYPE_ELEMENT(ETextureFormat::RG32F, 4),

    CONVERSION_TYPE_ELEMENT(ETextureFormat::D16, 2),
    CONVERSION_TYPE_ELEMENT(ETextureFormat::D16_S8, 3),
    CONVERSION_TYPE_ELEMENT(ETextureFormat::D24, 3),
    CONVERSION_TYPE_ELEMENT(ETextureFormat::D24_S8, 4),
    CONVERSION_TYPE_ELEMENT(ETextureFormat::D32, 4),
    CONVERSION_TYPE_ELEMENT(ETextureFormat::D32_S8, 5));
}

//GENERATE_CONVERSION_FUNCTION(GetDX12TextureDemension,
//  CONVERSION_TYPE_ELEMENT(ETextureType::TEXTURE_1D, D3D12_RESOURCE_DIMENSION_TEXTURE1D),
//  CONVERSION_TYPE_ELEMENT(ETextureType::TEXTURE_2D, D3D12_RESOURCE_DIMENSION_TEXTURE2D),
//  CONVERSION_TYPE_ELEMENT(ETextureType::TEXTURE_2D_ARRAY, D3D12_RESOURCE_DIMENSION_TEXTURE2D),
//  CONVERSION_TYPE_ELEMENT(ETextureType::TEXTURE_CUBE, D3D12_RESOURCE_DIMENSION_TEXTURE3D)) // CubeMap is Texture2D for DX12

// clang-format on

inline ETextureType GetDX12TextureDemension(D3D12_RESOURCE_DIMENSION InType,
                                            bool InIsArray) {
  switch (InType) {
    case D3D12_RESOURCE_DIMENSION_TEXTURE2D: {
      return InIsArray ? ETextureType::TEXTURE_2D_ARRAY
                       : ETextureType::TEXTURE_2D;
    }
    case D3D12_RESOURCE_DIMENSION_TEXTURE3D: {
      return InIsArray ? ETextureType::TEXTURE_3D_ARRAY
                       : ETextureType::TEXTURE_3D;
    }
    case D3D12_RESOURCE_DIMENSION_UNKNOWN:
    case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
    case D3D12_RESOURCE_DIMENSION_BUFFER:
    default:
      return ETextureType::TEXTURE_2D;
  }
}

inline D3D12_RESOURCE_DIMENSION GetDX12TextureDemension(ETextureType InType) {
  switch (InType) {
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

// clang-format off

GENERATE_CONVERSION_FUNCTION(GetDX12DescriptorHeapType,
  CONVERSION_TYPE_ELEMENT(EDescriptorHeapTypeDX12::CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV),
  CONVERSION_TYPE_ELEMENT(EDescriptorHeapTypeDX12::SAMPLER, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER),
  CONVERSION_TYPE_ELEMENT(EDescriptorHeapTypeDX12::RTV, D3D12_DESCRIPTOR_HEAP_TYPE_RTV),
  CONVERSION_TYPE_ELEMENT(EDescriptorHeapTypeDX12::DSV, D3D12_DESCRIPTOR_HEAP_TYPE_DSV))

GENERATE_CONVERSION_FUNCTION(GetDX12ShaderBindingType,
  CONVERSION_TYPE_ELEMENT(EShaderBindingType::UNIFORMBUFFER, D3D12_DESCRIPTOR_RANGE_TYPE_CBV),
  CONVERSION_TYPE_ELEMENT(EShaderBindingType::UNIFORMBUFFER_DYNAMIC, D3D12_DESCRIPTOR_RANGE_TYPE_CBV),
  CONVERSION_TYPE_ELEMENT(EShaderBindingType::TEXTURE_SAMPLER_SRV, D3D12_DESCRIPTOR_RANGE_TYPE_SRV),
  CONVERSION_TYPE_ELEMENT(EShaderBindingType::TEXTURE_SRV, D3D12_DESCRIPTOR_RANGE_TYPE_SRV),
  CONVERSION_TYPE_ELEMENT(EShaderBindingType::TEXTURE_UAV, D3D12_DESCRIPTOR_RANGE_TYPE_UAV),
  CONVERSION_TYPE_ELEMENT(EShaderBindingType::TEXTURE_ARRAY_SRV, D3D12_DESCRIPTOR_RANGE_TYPE_SRV),
  CONVERSION_TYPE_ELEMENT(EShaderBindingType::SAMPLER, D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER),
  CONVERSION_TYPE_ELEMENT(EShaderBindingType::BUFFER_SRV, D3D12_DESCRIPTOR_RANGE_TYPE_SRV),
  CONVERSION_TYPE_ELEMENT(EShaderBindingType::BUFFER_UAV, D3D12_DESCRIPTOR_RANGE_TYPE_UAV),
  CONVERSION_TYPE_ELEMENT(EShaderBindingType::BUFFER_UAV_DYNAMIC, D3D12_DESCRIPTOR_RANGE_TYPE_UAV),
  CONVERSION_TYPE_ELEMENT(EShaderBindingType::BUFFER_TEXEL_SRV, D3D12_DESCRIPTOR_RANGE_TYPE_SRV),
  CONVERSION_TYPE_ELEMENT(EShaderBindingType::BUFFER_TEXEL_UAV, D3D12_DESCRIPTOR_RANGE_TYPE_UAV),
  CONVERSION_TYPE_ELEMENT(EShaderBindingType::ACCELERATION_STRUCTURE_SRV, D3D12_DESCRIPTOR_RANGE_TYPE_SRV),
  CONVERSION_TYPE_ELEMENT(EShaderBindingType::SUBPASS_INPUT_ATTACHMENT, (D3D12_DESCRIPTOR_RANGE_TYPE)-1))

GENERATE_CONVERSION_FUNCTION(GetDX12TextureAddressMode,
  CONVERSION_TYPE_ELEMENT(ETextureAddressMode::REPEAT, D3D12_TEXTURE_ADDRESS_MODE_WRAP),
  CONVERSION_TYPE_ELEMENT(ETextureAddressMode::MIRRORED_REPEAT, D3D12_TEXTURE_ADDRESS_MODE_MIRROR),
  CONVERSION_TYPE_ELEMENT(ETextureAddressMode::CLAMP_TO_EDGE, D3D12_TEXTURE_ADDRESS_MODE_CLAMP),
  CONVERSION_TYPE_ELEMENT(ETextureAddressMode::CLAMP_TO_BORDER, D3D12_TEXTURE_ADDRESS_MODE_BORDER),
  CONVERSION_TYPE_ELEMENT(ETextureAddressMode::MIRROR_CLAMP_TO_EDGE, D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE))

GENERATE_CONVERSION_FUNCTION(GetDX12CompareOp,
  CONVERSION_TYPE_ELEMENT(ECompareOp::NEVER, D3D12_COMPARISON_FUNC_NEVER),
  CONVERSION_TYPE_ELEMENT(ECompareOp::LESS, D3D12_COMPARISON_FUNC_LESS),
  CONVERSION_TYPE_ELEMENT(ECompareOp::EQUAL, D3D12_COMPARISON_FUNC_EQUAL),
  CONVERSION_TYPE_ELEMENT(ECompareOp::LEQUAL, D3D12_COMPARISON_FUNC_LESS_EQUAL),
  CONVERSION_TYPE_ELEMENT(ECompareOp::GREATER, D3D12_COMPARISON_FUNC_GREATER),
  CONVERSION_TYPE_ELEMENT(ECompareOp::NOTEQUAL, D3D12_COMPARISON_FUNC_NOT_EQUAL),
  CONVERSION_TYPE_ELEMENT(ECompareOp::GEQUAL, D3D12_COMPARISON_FUNC_GREATER_EQUAL),
  CONVERSION_TYPE_ELEMENT(ECompareOp::ALWAYS, D3D12_COMPARISON_FUNC_ALWAYS))

GENERATE_CONVERSION_FUNCTION(GetDX12StencilOp,
  CONVERSION_TYPE_ELEMENT(EStencilOp::KEEP, D3D12_STENCIL_OP_KEEP),
  CONVERSION_TYPE_ELEMENT(EStencilOp::ZERO, D3D12_STENCIL_OP_ZERO),
  CONVERSION_TYPE_ELEMENT(EStencilOp::REPLACE, D3D12_STENCIL_OP_REPLACE),
  CONVERSION_TYPE_ELEMENT(EStencilOp::INCR, D3D12_STENCIL_OP_INCR_SAT),
  CONVERSION_TYPE_ELEMENT(EStencilOp::INCR_WRAP, D3D12_STENCIL_OP_INCR),
  CONVERSION_TYPE_ELEMENT(EStencilOp::DECR, D3D12_STENCIL_OP_DECR_SAT),
  CONVERSION_TYPE_ELEMENT(EStencilOp::DECR_WRAP, D3D12_STENCIL_OP_DECR),
  CONVERSION_TYPE_ELEMENT(EStencilOp::INVERT, D3D12_STENCIL_OP_INVERT))

inline auto GetDX12PrimitiveTopologyTypeOnly(EPrimitiveType type) {
  GENERATE_STATIC_CONVERSION_ARRAY(
    CONVERSION_TYPE_ELEMENT(EPrimitiveType::POINTS, D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT),
    CONVERSION_TYPE_ELEMENT(EPrimitiveType::LINES, D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE),
    CONVERSION_TYPE_ELEMENT(EPrimitiveType::LINES_ADJACENCY, D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE),
    CONVERSION_TYPE_ELEMENT(EPrimitiveType::LINE_STRIP_ADJACENCY, D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE),
    CONVERSION_TYPE_ELEMENT(EPrimitiveType::TRIANGLES, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE),
    CONVERSION_TYPE_ELEMENT(EPrimitiveType::TRIANGLE_STRIP, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE),
    CONVERSION_TYPE_ELEMENT(EPrimitiveType::TRIANGLES_ADJACENCY, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE),
    CONVERSION_TYPE_ELEMENT(EPrimitiveType::TRIANGLE_STRIP_ADJACENCY, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE));
}

inline auto GetDX12PrimitiveTopology(EPrimitiveType type) {
  GENERATE_STATIC_CONVERSION_ARRAY(
    CONVERSION_TYPE_ELEMENT(EPrimitiveType::POINTS, D3D_PRIMITIVE_TOPOLOGY_POINTLIST),
    CONVERSION_TYPE_ELEMENT(EPrimitiveType::LINES, D3D_PRIMITIVE_TOPOLOGY_LINELIST),
    CONVERSION_TYPE_ELEMENT(EPrimitiveType::LINES_ADJACENCY, D3D_PRIMITIVE_TOPOLOGY_LINELIST_ADJ),
    CONVERSION_TYPE_ELEMENT(EPrimitiveType::LINE_STRIP_ADJACENCY, D3D_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ),
    CONVERSION_TYPE_ELEMENT(EPrimitiveType::TRIANGLES, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST),
    CONVERSION_TYPE_ELEMENT(EPrimitiveType::TRIANGLE_STRIP, D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP),
    CONVERSION_TYPE_ELEMENT(EPrimitiveType::TRIANGLES_ADJACENCY, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ),
    CONVERSION_TYPE_ELEMENT(EPrimitiveType::TRIANGLE_STRIP_ADJACENCY, D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ));
}

GENERATE_CONVERSION_FUNCTION(GetDX12VertexInputRate,
  CONVERSION_TYPE_ELEMENT(EVertexInputRate::VERTEX, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA),
  CONVERSION_TYPE_ELEMENT(EVertexInputRate::INSTANCE, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA))

GENERATE_CONVERSION_FUNCTION(GetDX12PolygonMode,
  CONVERSION_TYPE_ELEMENT(EPolygonMode::POINT, D3D12_FILL_MODE_WIREFRAME), // Not supported
  CONVERSION_TYPE_ELEMENT(EPolygonMode::LINE, D3D12_FILL_MODE_WIREFRAME),
  CONVERSION_TYPE_ELEMENT(EPolygonMode::FILL, D3D12_FILL_MODE_SOLID))

GENERATE_CONVERSION_FUNCTION(GetDX12CullMode,
  CONVERSION_TYPE_ELEMENT(ECullMode::NONE, D3D12_CULL_MODE_NONE),
  CONVERSION_TYPE_ELEMENT(ECullMode::BACK, D3D12_CULL_MODE_BACK),
  CONVERSION_TYPE_ELEMENT(ECullMode::FRONT, D3D12_CULL_MODE_FRONT),
  CONVERSION_TYPE_ELEMENT(ECullMode::FRONT_AND_BACK, D3D12_CULL_MODE_BACK)) // Not supported

inline auto GetDX12BlendFactor(EBlendFactor type) {
  GENERATE_STATIC_CONVERSION_ARRAY(
    CONVERSION_TYPE_ELEMENT(EBlendFactor::ZERO, D3D12_BLEND_ZERO),
    CONVERSION_TYPE_ELEMENT(EBlendFactor::ONE, D3D12_BLEND_ONE),
    CONVERSION_TYPE_ELEMENT(EBlendFactor::SRC_COLOR, D3D12_BLEND_SRC_COLOR),
    CONVERSION_TYPE_ELEMENT(EBlendFactor::ONE_MINUS_SRC_COLOR, D3D12_BLEND_INV_SRC_COLOR),
    CONVERSION_TYPE_ELEMENT(EBlendFactor::DST_COLOR, D3D12_BLEND_DEST_COLOR),
    CONVERSION_TYPE_ELEMENT(EBlendFactor::ONE_MINUS_DST_COLOR, D3D12_BLEND_INV_DEST_COLOR),
    CONVERSION_TYPE_ELEMENT(EBlendFactor::SRC_ALPHA, D3D12_BLEND_SRC_ALPHA),
    CONVERSION_TYPE_ELEMENT(EBlendFactor::ONE_MINUS_SRC_ALPHA, D3D12_BLEND_INV_SRC_ALPHA),
    CONVERSION_TYPE_ELEMENT(EBlendFactor::DST_ALPHA, D3D12_BLEND_DEST_ALPHA),
    CONVERSION_TYPE_ELEMENT(EBlendFactor::ONE_MINUS_DST_ALPHA, D3D12_BLEND_INV_DEST_ALPHA),
    CONVERSION_TYPE_ELEMENT(EBlendFactor::CONSTANT_COLOR, D3D12_BLEND_BLEND_FACTOR),
    CONVERSION_TYPE_ELEMENT(EBlendFactor::ONE_MINUS_CONSTANT_COLOR, D3D12_BLEND_INV_BLEND_FACTOR),
    // TODO: either comment out or implement these
    CONVERSION_TYPE_ELEMENT(EBlendFactor::CONSTANT_ALPHA, D3D12_BLEND_ZERO),           // Not supported in DX12.
    CONVERSION_TYPE_ELEMENT(EBlendFactor::ONE_MINUS_CONSTANT_ALPHA, D3D12_BLEND_ZERO), // Not supported in DX12.
    CONVERSION_TYPE_ELEMENT(EBlendFactor::SRC_ALPHA_SATURATE, D3D12_BLEND_SRC_ALPHA_SAT));
}

GENERATE_CONVERSION_FUNCTION(GetDX12BlendOp,
  CONVERSION_TYPE_ELEMENT(EBlendOp::ADD, D3D12_BLEND_OP_ADD),
  CONVERSION_TYPE_ELEMENT(EBlendOp::SUBTRACT, D3D12_BLEND_OP_SUBTRACT),
  CONVERSION_TYPE_ELEMENT(EBlendOp::REVERSE_SUBTRACT, D3D12_BLEND_OP_REV_SUBTRACT),
  CONVERSION_TYPE_ELEMENT(EBlendOp::MIN_VALUE, D3D12_BLEND_OP_MIN),
  CONVERSION_TYPE_ELEMENT(EBlendOp::MAX_VALUE, D3D12_BLEND_OP_MAX))

// clang-format off

// clang-format on

inline uint8_t GetDX12ColorMask(EColorMask type) {
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

inline D3D12_FILTER GetDX12TextureFilter(ETextureFilter InMinification,
                                         ETextureFilter InMagnification,
                                         bool InIsComparison = false) {
  // Comparison is used for ShadowMap
  if (InIsComparison) {
    D3D12_FILTER Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
    switch (InMinification) {
      case ETextureFilter::NEAREST:
      case ETextureFilter::NEAREST_MIPMAP_NEAREST:
        if (InMagnification == ETextureFilter::NEAREST) {
          Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
        } else if (InMagnification == ETextureFilter::LINEAR) {
          Filter = D3D12_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT;
        } else if (InMagnification == ETextureFilter::NEAREST_MIPMAP_NEAREST) {
          Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
        } else if (InMagnification == ETextureFilter::LINEAR_MIPMAP_NEAREST) {
          Filter = D3D12_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT;
        } else if (InMagnification == ETextureFilter::NEAREST_MIPMAP_LINEAR) {
          Filter = D3D12_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR;
        } else if (InMagnification == ETextureFilter::LINEAR_MIPMAP_LINEAR) {
          Filter = D3D12_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR;
        }
        break;
      case ETextureFilter::LINEAR:
      case ETextureFilter::LINEAR_MIPMAP_NEAREST:
        if (InMagnification == ETextureFilter::NEAREST) {
          Filter = D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT;
        } else if (InMagnification == ETextureFilter::LINEAR) {
          Filter = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
        } else if (InMagnification == ETextureFilter::NEAREST_MIPMAP_NEAREST) {
          Filter = D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT;
        } else if (InMagnification == ETextureFilter::LINEAR_MIPMAP_NEAREST) {
          Filter = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
        } else if (InMagnification == ETextureFilter::NEAREST_MIPMAP_LINEAR) {
          Filter = D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
        } else if (InMagnification == ETextureFilter::LINEAR_MIPMAP_LINEAR) {
          Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
        }
        break;
      case ETextureFilter::NEAREST_MIPMAP_LINEAR:
        if (InMagnification == ETextureFilter::NEAREST) {
          Filter = D3D12_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR;
        } else if (InMagnification == ETextureFilter::LINEAR) {
          Filter = D3D12_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR;
        } else if (InMagnification == ETextureFilter::NEAREST_MIPMAP_NEAREST) {
          // If mipmaps overlap, upload them as linear and process them. Needs
          // organization.
          Filter = D3D12_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR;
        } else if (InMagnification == ETextureFilter::LINEAR_MIPMAP_NEAREST) {
          // If mipmaps overlap, upload them as linear and process them. Needs
          // organization.
          Filter = D3D12_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR;
        } else if (InMagnification == ETextureFilter::NEAREST_MIPMAP_LINEAR) {
          Filter = D3D12_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR;
        } else if (InMagnification == ETextureFilter::LINEAR_MIPMAP_LINEAR) {
          Filter = D3D12_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR;
        }
        break;
      case ETextureFilter::LINEAR_MIPMAP_LINEAR:
        if (InMagnification == ETextureFilter::NEAREST) {
          Filter = D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
        } else if (InMagnification == ETextureFilter::LINEAR) {
          Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
        } else if (InMagnification == ETextureFilter::NEAREST_MIPMAP_NEAREST) {
          // If mipmaps overlap, upload them as linear and process them. Needs
          // organization.
          Filter = D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
        } else if (InMagnification == ETextureFilter::LINEAR_MIPMAP_NEAREST) {
          // If mipmaps overlap, upload them as linear and process them. Needs
          // organization.
          Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
        } else if (InMagnification == ETextureFilter::NEAREST_MIPMAP_LINEAR) {
          Filter = D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
        } else if (InMagnification == ETextureFilter::LINEAR_MIPMAP_LINEAR) {
          Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
        }
        break;
      default:
        break;
    }
    return Filter;
  } else {
    D3D12_FILTER Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
    switch (InMinification) {
      case ETextureFilter::NEAREST:
      case ETextureFilter::NEAREST_MIPMAP_NEAREST:
        if (InMagnification == ETextureFilter::NEAREST) {
          Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
        } else if (InMagnification == ETextureFilter::LINEAR) {
          Filter = D3D12_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
        } else if (InMagnification == ETextureFilter::NEAREST_MIPMAP_NEAREST) {
          Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
        } else if (InMagnification == ETextureFilter::LINEAR_MIPMAP_NEAREST) {
          Filter = D3D12_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
        } else if (InMagnification == ETextureFilter::NEAREST_MIPMAP_LINEAR) {
          Filter = D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR;
        } else if (InMagnification == ETextureFilter::LINEAR_MIPMAP_LINEAR) {
          Filter = D3D12_FILTER_MIN_POINT_MAG_MIP_LINEAR;
        }
        break;
      case ETextureFilter::LINEAR:
      case ETextureFilter::LINEAR_MIPMAP_NEAREST:
        if (InMagnification == ETextureFilter::NEAREST) {
          Filter = D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT;
        } else if (InMagnification == ETextureFilter::LINEAR) {
          Filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
        } else if (InMagnification == ETextureFilter::NEAREST_MIPMAP_NEAREST) {
          Filter = D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT;
        } else if (InMagnification == ETextureFilter::LINEAR_MIPMAP_NEAREST) {
          Filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
        } else if (InMagnification == ETextureFilter::NEAREST_MIPMAP_LINEAR) {
          Filter = D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
        } else if (InMagnification == ETextureFilter::LINEAR_MIPMAP_LINEAR) {
          Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
        }
        break;
      case ETextureFilter::NEAREST_MIPMAP_LINEAR:
        if (InMagnification == ETextureFilter::NEAREST) {
          Filter = D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR;
        } else if (InMagnification == ETextureFilter::LINEAR) {
          Filter = D3D12_FILTER_MIN_POINT_MAG_MIP_LINEAR;
        } else if (InMagnification == ETextureFilter::NEAREST_MIPMAP_NEAREST) {
          // If mipmaps overlap, upload them as linear and process them. Needs
          // organization.
          Filter = D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR;
        } else if (InMagnification == ETextureFilter::LINEAR_MIPMAP_NEAREST) {
          // If mipmaps overlap, upload them as linear and process them. Needs
          // organization.
          Filter = D3D12_FILTER_MIN_POINT_MAG_MIP_LINEAR;
        } else if (InMagnification == ETextureFilter::NEAREST_MIPMAP_LINEAR) {
          Filter = D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR;
        } else if (InMagnification == ETextureFilter::LINEAR_MIPMAP_LINEAR) {
          Filter = D3D12_FILTER_MIN_POINT_MAG_MIP_LINEAR;
        }
        break;
      case ETextureFilter::LINEAR_MIPMAP_LINEAR:
        if (InMagnification == ETextureFilter::NEAREST) {
          Filter = D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
        } else if (InMagnification == ETextureFilter::LINEAR) {
          Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
        } else if (InMagnification == ETextureFilter::NEAREST_MIPMAP_NEAREST) {
          // If mipmaps overlap, upload them as linear and process them. Needs
          // organization.
          Filter = D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
        } else if (InMagnification == ETextureFilter::LINEAR_MIPMAP_NEAREST) {
          // If mipmaps overlap, upload them as linear and process them. Needs
          // organization.
          Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
        } else if (InMagnification == ETextureFilter::NEAREST_MIPMAP_LINEAR) {
          Filter = D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
        } else if (InMagnification == ETextureFilter::LINEAR_MIPMAP_LINEAR) {
          Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
        }
        break;
      default:
        break;
    }
    return Filter;
  }
}

inline void GetDepthFormatForSRV(DXGI_FORMAT& OutTexFormat,
                                 DXGI_FORMAT& OutSrvFormat,
                                 DXGI_FORMAT  InOriginalTexFormat) {
  switch (InOriginalTexFormat) {
    case DXGI_FORMAT_D16_UNORM:
      OutTexFormat = DXGI_FORMAT_R16_TYPELESS;
      OutSrvFormat = DXGI_FORMAT_R16_UNORM;
      break;
    case DXGI_FORMAT_D24_UNORM_S8_UINT:
      OutTexFormat = DXGI_FORMAT_R24G8_TYPELESS;
      OutSrvFormat = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
      break;
    case DXGI_FORMAT_D32_FLOAT:
      OutTexFormat = DXGI_FORMAT_R32_TYPELESS;
      OutSrvFormat = DXGI_FORMAT_R32_FLOAT;
      break;
    case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
      OutTexFormat = DXGI_FORMAT_R32G8X24_TYPELESS;
      OutSrvFormat = DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
      break;
    default:
      assert(0);
      break;
  }
}

// clang-format off

GENERATE_CONVERSION_FUNCTION(GetDX12ResourceLayout,
  CONVERSION_TYPE_ELEMENT(EResourceLayout::UNDEFINED, D3D12_RESOURCE_STATE_COMMON),
  CONVERSION_TYPE_ELEMENT(EResourceLayout::GENERAL, D3D12_RESOURCE_STATE_COMMON),
  CONVERSION_TYPE_ELEMENT(EResourceLayout::UAV, D3D12_RESOURCE_STATE_UNORDERED_ACCESS),
  CONVERSION_TYPE_ELEMENT(EResourceLayout::COLOR_ATTACHMENT, D3D12_RESOURCE_STATE_RENDER_TARGET),
  CONVERSION_TYPE_ELEMENT(EResourceLayout::DEPTH_STENCIL_ATTACHMENT, D3D12_RESOURCE_STATE_DEPTH_WRITE),
  CONVERSION_TYPE_ELEMENT(EResourceLayout::DEPTH_STENCIL_READ_ONLY, D3D12_RESOURCE_STATE_DEPTH_READ),
  CONVERSION_TYPE_ELEMENT(EResourceLayout::SHADER_READ_ONLY, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE 
                                                             | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
  CONVERSION_TYPE_ELEMENT(EResourceLayout::TRANSFER_SRC, D3D12_RESOURCE_STATE_COPY_SOURCE),
  CONVERSION_TYPE_ELEMENT(EResourceLayout::TRANSFER_DST, D3D12_RESOURCE_STATE_COPY_DEST),
  CONVERSION_TYPE_ELEMENT(EResourceLayout::PREINITIALIZED, D3D12_RESOURCE_STATE_COMMON),
  CONVERSION_TYPE_ELEMENT(EResourceLayout::DEPTH_READ_ONLY_STENCIL_ATTACHMENT, D3D12_RESOURCE_STATE_DEPTH_WRITE),
  CONVERSION_TYPE_ELEMENT(EResourceLayout::DEPTH_ATTACHMENT_STENCIL_READ_ONLY, D3D12_RESOURCE_STATE_DEPTH_WRITE),
  CONVERSION_TYPE_ELEMENT(EResourceLayout::DEPTH_ATTACHMENT, D3D12_RESOURCE_STATE_DEPTH_WRITE),
  CONVERSION_TYPE_ELEMENT(EResourceLayout::DEPTH_READ_ONLY, D3D12_RESOURCE_STATE_DEPTH_READ 
                                                            | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE 
                                                            | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
  CONVERSION_TYPE_ELEMENT(EResourceLayout::STENCIL_ATTACHMENT, D3D12_RESOURCE_STATE_DEPTH_WRITE),
  CONVERSION_TYPE_ELEMENT(EResourceLayout::STENCIL_READ_ONLY, D3D12_RESOURCE_STATE_DEPTH_READ 
                                                              | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE 
                                                              | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
  CONVERSION_TYPE_ELEMENT(EResourceLayout::PRESENT_SRC, D3D12_RESOURCE_STATE_PRESENT),
  CONVERSION_TYPE_ELEMENT(EResourceLayout::SHARED_PRESENT, D3D12_RESOURCE_STATE_PRESENT),
  CONVERSION_TYPE_ELEMENT(EResourceLayout::SHADING_RATE_NV, D3D12_RESOURCE_STATE_SHADING_RATE_SOURCE),
  CONVERSION_TYPE_ELEMENT(EResourceLayout::FRAGMENT_DENSITY_MAP_EXT, D3D12_RESOURCE_STATE_COMMON),
  CONVERSION_TYPE_ELEMENT(EResourceLayout::READ_ONLY, D3D12_RESOURCE_STATE_GENERIC_READ),
  CONVERSION_TYPE_ELEMENT(EResourceLayout::ATTACHMENT, D3D12_RESOURCE_STATE_RENDER_TARGET),
  CONVERSION_TYPE_ELEMENT(EResourceLayout::ACCELERATION_STRUCTURE, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE))

// clang-format on

// Generated from CreateResource, CreateUploadResource
struct jCreatedResource
    : public std::enable_shared_from_this<jCreatedResource> {
  ~jCreatedResource() { Free(); }

  enum class EType : uint8_t {
    Standalone,    // CommittedResource
    ResourcePool,  // PlacedResource
    Swapchain,     // no need release by me
  };

  static std::shared_ptr<jCreatedResource> CreatedFromStandalone(
      const ComPtr<ID3D12Resource>& InResource) {
    return std::shared_ptr<jCreatedResource>(
        new jCreatedResource(EType::Standalone, InResource));
  }

  static std::shared_ptr<jCreatedResource> CreatedFromResourcePool(
      const ComPtr<ID3D12Resource>& InResource) {
    return std::shared_ptr<jCreatedResource>(
        new jCreatedResource(EType::ResourcePool, InResource));
  }

  static std::shared_ptr<jCreatedResource> CreatedFromSwapchain(
      const ComPtr<ID3D12Resource>& InResource) {
    return std::shared_ptr<jCreatedResource>(
        new jCreatedResource(EType::Swapchain, InResource));
  }

  EType                                   ResourceType = EType::Standalone;
  // TODO: consider remove shared ptr and use pure Resource
  std::shared_ptr<ComPtr<ID3D12Resource>> Resource;

  bool IsValid() const { return Resource && (*Resource).Get(); }

  ID3D12Resource* Get() const { return Resource ? (*Resource).Get() : nullptr; }

  const ComPtr<ID3D12Resource>& GetPtr() const { return *Resource; }

  uint64_t GetGPUVirtualAddress() const {
    return Resource ? (*Resource)->GetGPUVirtualAddress() : 0;
  }

  void Free();

  private:
  jCreatedResource() {}

  jCreatedResource(EType InType, const ComPtr<ID3D12Resource>& InResource)
      : ResourceType(InType)
      , Resource(std::make_shared<ComPtr<ID3D12Resource>>(InResource)) {}

  // prevent copying
  jCreatedResource(const jCreatedResource&)            = delete;
  jCreatedResource& operator=(const jCreatedResource&) = delete;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_RHI_TYPE_DX12_H