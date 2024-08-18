#ifndef GAME_ENGINE_RHI_TYPE_H
#define GAME_ENGINE_RHI_TYPE_H

#include "gfx/rhi/instant_struct.h"
#include "utils/enum/enum_util.h"

#include <math_library/graphics.h>
#include <math_library/vector.h>

#include <array>
#include <sstream>
#include <string>
#include <type_traits>
#include <unordered_map>

// TODO:
// - rename functions that GENERATE_CONVERSION_FUNCTION creates to correct
// naming convention
// - consider whether there's better solution to generating convertion function
// exists (to reduce compile time, etc.)

namespace game_engine {

//////////////////////////////////////////////////////////////////////////
// Auto generate type conversion code
template <typename T1, typename T2>
using ConversionTypePair = std::pair<T1, T2>;

// Get the first type of Parameter Packs
template <typename T, typename... T1>
using PacksType = T;

template <typename... T1>
constexpr auto GenerateConversionTypeArray(T1... args) {
  // Obtain the second_type of the first type in a parameter pack made of
  // std::pairs
  using value_type = typename PacksType<T1...>::second_type;

  std::array<value_type, sizeof...(args)> newArray;
  auto addElementFunc = [&newArray](const auto& arg) {
    newArray[(int64_t)arg.first] = arg.second;
  };

  int dummy[] = {0, (addElementFunc(args), 0)...};
  return newArray;
}

template <typename... T1>
constexpr auto GenerateConversionTypeMap(T1... args) {
  // Obtain the first_type and second_type from the first type in a parameter
  // pack made of std::pairs
  using key_type   = typename PacksType<T1...>::first_type;
  using value_type = typename PacksType<T1...>::second_type;

  std::unordered_map<key_type, value_type> newMap;
  auto addElementFunc = [&newMap](const auto& arg) { newMap.insert(arg); };

  int dummy[] = {0, (addElementFunc(args), 0)...};
  return newMap;
}

template <typename... T1>
constexpr auto GenerateInverseConversionTypeMap(T1... args) {
  // Obtain the second_type as key and first_type as value from the first type
  // in a parameter pack made of std::pairs
  using key_type   = typename PacksType<T1...>::second_type;
  using value_type = typename PacksType<T1...>::first_type;

  std::unordered_map<key_type, value_type> newMap;
  auto                                     addElementFunc
      = [&newMap](const auto& arg) { newMap[arg.second] = arg.first; };

  int dummy[] = {0, (addElementFunc(args), 0)...};
  return newMap;
}

#define CONVERSION_TYPE_ELEMENT(x, y) \
  ConversionTypePair<decltype(x), decltype(y)>(x, y)

// Macro to insert the given elements into an array. This is used for converting
// from engine types to API types. It's suitable for arrays because the range of
// engine types is small and all integer values from 0 to N are used.
#define GENERATE_STATIC_CONVERSION_ARRAY(...)                           \
  {                                                                     \
    static auto _TypeArray_ = GenerateConversionTypeArray(__VA_ARGS__); \
    return _TypeArray_[(int64_t)type];                                  \
  }

// Macro to insert the given elements into a map. This is used for converting
// from API types to engine types. Maps are used here because the range of
// engine types can be large or mixed, making arrays inefficient due to memory
// waste.
#define GENERATE_STATIC_CONVERSION_MAP(...)                         \
  {                                                                 \
    static auto _TypeMap_ = GenerateConversionTypeMap(__VA_ARGS__); \
    return _TypeMap_[type];                                         \
  }
#define GENERATE_STATIC_INVERSECONVERSION_MAP(...)                         \
  {                                                                        \
    static auto _TypeMap_ = GenerateInverseConversionTypeMap(__VA_ARGS__); \
    return _TypeMap_[type];                                                \
  }

// Macro to obtain the first item of a variadic macro
#define DEDUCE_FIRST(First, ...) First

// Macro to generate functions for converting between engine types and API types
#define GENERATE_CONVERSION_FUNCTION(FunctionName, ...)          \
  using FunctionName##key_type =                                 \
      typename decltype(DEDUCE_FIRST(__VA_ARGS__))::first_type;  \
  using FunctionName##value_type =                               \
      typename decltype(DEDUCE_FIRST(__VA_ARGS__))::second_type; \
  inline auto FunctionName(FunctionName##key_type type) {        \
    GENERATE_STATIC_CONVERSION_ARRAY(__VA_ARGS__)                \
  }                                                              \
  inline auto FunctionName(FunctionName##value_type type) {      \
    GENERATE_STATIC_INVERSECONVERSION_MAP(__VA_ARGS__)           \
  }

template <typename EnumType>
std::array<std::string, static_cast<size_t>(EnumType::MAX) + 1> split(
    const std::string& s, char delim) {
  std::array<std::string, static_cast<size_t>(EnumType::MAX) + 1> result;
  std::stringstream                                               ss(s);
  std::string                                                     item;

  int32_t Count = 0;
  while (getline(ss, item, delim)) {
    result[Count++] = item;
  }

  return result;
}

#define MAKE_ENUM_TO_STRING_CONT(EnumType, N, EnumListString) \
  static std::array<const char*, N> EnumType##Strings         \
      = split(EnumListString, ',');

// Macro to declare an enum class and generate a function for converting enum
// values to strings
#define DECLARE_ENUM_WITH_CONVERT_TO_STRING(EnumType, UnderlyingType, ...)   \
  enum class EnumType : UnderlyingType {                                     \
    __VA_ARGS__                                                              \
  };                                                                         \
  static std::array<std::string, static_cast<size_t>(EnumType::MAX) + 1>     \
                     EnumType##Strings = split<EnumType>(#__VA_ARGS__, ','); \
  inline const char* EnumToString(EnumType value) {                          \
    return EnumType##Strings[static_cast<size_t>(value)].c_str();            \
  }
//////////////////////////////////////////////////////////////////////////

DECLARE_ENUM_WITH_CONVERT_TO_STRING(EPrimitiveType,
                                    uint8_t,
                                    POINTS,
                                    LINES,
                                    LINES_ADJACENCY,
                                    LINE_STRIP_ADJACENCY,
                                    TRIANGLES,
                                    TRIANGLE_STRIP,
                                    TRIANGLES_ADJACENCY,
                                    TRIANGLE_STRIP_ADJACENCY,
                                    MAX);

DECLARE_ENUM_WITH_CONVERT_TO_STRING(
    EVertexInputRate, uint8_t, VERTEX, INSTANCE, MAX);

DECLARE_ENUM_WITH_CONVERT_TO_STRING(EBufferType, uint8_t, Static, Dynamic, MAX);

DECLARE_ENUM_WITH_CONVERT_TO_STRING(EBufferElementType,
                                    uint8_t,
                                    BYTE,
                                    BYTE_UNORM,
                                    UINT16,
                                    UINT32,
                                    FLOAT,
                                    MAX, );

enum class ETextureFilterTarget : uint8_t {
  MINIFICATION = 0,
  MAGNIFICATION,
  MAX
};

DECLARE_ENUM_WITH_CONVERT_TO_STRING(ETextureFilter,
                                    uint8_t,
                                    NEAREST,
                                    LINEAR,
                                    NEAREST_MIPMAP_NEAREST,
                                    LINEAR_MIPMAP_NEAREST,
                                    NEAREST_MIPMAP_LINEAR,
                                    LINEAR_MIPMAP_LINEAR,
                                    MAX);

DECLARE_ENUM_WITH_CONVERT_TO_STRING(ETextureType,
                                    uint8_t,
                                    TEXTURE_1D,
                                    TEXTURE_2D,
                                    TEXTURE_2D_ARRAY,
                                    TEXTURE_3D,
                                    TEXTURE_3D_ARRAY,
                                    TEXTURE_CUBE,
                                    MAX, );

DECLARE_ENUM_WITH_CONVERT_TO_STRING(ETextureFormat,
                                    uint8_t,
                                    // color
                                    RGB8,
                                    RGB16F,
                                    RGB32F,
                                    R11G11B10F,

                                    RGBA8,
                                    RGBA16F,
                                    RGBA32F,
                                    RGBA8SI,
                                    RGBA8UI,

                                    BGRA8,

                                    R8,
                                    R16F,
                                    R32F,
                                    R8UI,
                                    R32UI,

                                    RG8,
                                    RG16F,
                                    RG32F,

                                    // depth
                                    D16,
                                    D16_S8,
                                    D24,
                                    D24_S8,
                                    D32,
                                    D32_S8,

                                    BC1_UNORM,
                                    BC2_UNORM,
                                    BC3_UNORM,
                                    BC4_UNORM,
                                    BC4_SNORM,
                                    BC5_UNORM,
                                    BC5_SNORM,
                                    BC6H_UF16,
                                    BC6H_SF16,
                                    BC7_UNORM,
                                    MAX, );

static bool IsDepthFormat(ETextureFormat format) {
  switch (format) {
    case ETextureFormat::D16:
    case ETextureFormat::D16_S8:
    case ETextureFormat::D24:
    case ETextureFormat::D24_S8:
    case ETextureFormat::D32:
    case ETextureFormat::D32_S8:
      return true;
  }

  return false;
}

static bool IsDepthOnlyFormat(ETextureFormat format) {
  switch (format) {
    case ETextureFormat::D16:
    case ETextureFormat::D24:
    case ETextureFormat::D32:
      return true;
  }

  return false;
}

DECLARE_ENUM_WITH_CONVERT_TO_STRING(EFormatType,
                                    uint8_t,
                                    BYTE,
                                    UNSIGNED_BYTE,
                                    SHORT_INT,
                                    INT,
                                    UNSIGNED_INT,
                                    HALF,
                                    FLOAT,
                                    MAX);

// clang-format off

GENERATE_CONVERSION_FUNCTION(GetTexturePixelType,
    CONVERSION_TYPE_ELEMENT(ETextureFormat::RGB8, EFormatType::UNSIGNED_BYTE),          // not support rgb8 -> rgba8
    CONVERSION_TYPE_ELEMENT(ETextureFormat::RGB32F, EFormatType::HALF),                 // not support rgb32 -> rgba32
    CONVERSION_TYPE_ELEMENT(ETextureFormat::RGB16F, EFormatType::HALF),                 // not support rgb16 -> rgba16
    CONVERSION_TYPE_ELEMENT(ETextureFormat::R11G11B10F, EFormatType::HALF),
    CONVERSION_TYPE_ELEMENT(ETextureFormat::RGBA8, EFormatType::UNSIGNED_BYTE),
    CONVERSION_TYPE_ELEMENT(ETextureFormat::RGBA16F, EFormatType::HALF),
    CONVERSION_TYPE_ELEMENT(ETextureFormat::RGBA32F, EFormatType::FLOAT),
    CONVERSION_TYPE_ELEMENT(ETextureFormat::RGBA8SI, EFormatType::INT),
    CONVERSION_TYPE_ELEMENT(ETextureFormat::RGBA8UI, EFormatType::INT),
    CONVERSION_TYPE_ELEMENT(ETextureFormat::BGRA8, EFormatType::UNSIGNED_BYTE),
    CONVERSION_TYPE_ELEMENT(ETextureFormat::R8, EFormatType::UNSIGNED_BYTE),
    CONVERSION_TYPE_ELEMENT(ETextureFormat::R16F, EFormatType::HALF),
    CONVERSION_TYPE_ELEMENT(ETextureFormat::R32F, EFormatType::FLOAT),
    CONVERSION_TYPE_ELEMENT(ETextureFormat::R8UI, EFormatType::INT),
    CONVERSION_TYPE_ELEMENT(ETextureFormat::R32UI, EFormatType::INT),
    CONVERSION_TYPE_ELEMENT(ETextureFormat::RG8, EFormatType::UNSIGNED_BYTE),
    CONVERSION_TYPE_ELEMENT(ETextureFormat::RG16F, EFormatType::HALF),
    CONVERSION_TYPE_ELEMENT(ETextureFormat::RG32F, EFormatType::FLOAT),
    CONVERSION_TYPE_ELEMENT(ETextureFormat::D16, EFormatType::SHORT_INT),
    CONVERSION_TYPE_ELEMENT(ETextureFormat::D16_S8, EFormatType::INT),                  // not support d16_s8 -> d24_s8
    CONVERSION_TYPE_ELEMENT(ETextureFormat::D24, EFormatType::INT),
    CONVERSION_TYPE_ELEMENT(ETextureFormat::D24_S8, EFormatType::INT),
    CONVERSION_TYPE_ELEMENT(ETextureFormat::D32, EFormatType::FLOAT),
    CONVERSION_TYPE_ELEMENT(ETextureFormat::D32_S8, EFormatType::FLOAT),
    CONVERSION_TYPE_ELEMENT(ETextureFormat::BC1_UNORM, EFormatType::UNSIGNED_BYTE),
    CONVERSION_TYPE_ELEMENT(ETextureFormat::BC2_UNORM, EFormatType::UNSIGNED_BYTE),
    CONVERSION_TYPE_ELEMENT(ETextureFormat::BC3_UNORM, EFormatType::UNSIGNED_BYTE),
    CONVERSION_TYPE_ELEMENT(ETextureFormat::BC4_UNORM, EFormatType::UNSIGNED_BYTE),
    CONVERSION_TYPE_ELEMENT(ETextureFormat::BC4_SNORM, EFormatType::UNSIGNED_BYTE),
    CONVERSION_TYPE_ELEMENT(ETextureFormat::BC5_UNORM, EFormatType::UNSIGNED_BYTE),
    CONVERSION_TYPE_ELEMENT(ETextureFormat::BC5_SNORM, EFormatType::UNSIGNED_BYTE),
    CONVERSION_TYPE_ELEMENT(ETextureFormat::BC6H_UF16, EFormatType::HALF),
    CONVERSION_TYPE_ELEMENT(ETextureFormat::BC6H_SF16, EFormatType::HALF),
    CONVERSION_TYPE_ELEMENT(ETextureFormat::BC7_UNORM, EFormatType::HALF)
)

// clang-format on

DECLARE_ENUM_WITH_CONVERT_TO_STRING(EBlendFactor,
                                    uint8_t,
                                    ZERO,
                                    ONE,
                                    SRC_COLOR,
                                    ONE_MINUS_SRC_COLOR,
                                    DST_COLOR,
                                    ONE_MINUS_DST_COLOR,
                                    SRC_ALPHA,
                                    ONE_MINUS_SRC_ALPHA,
                                    DST_ALPHA,
                                    ONE_MINUS_DST_ALPHA,
                                    CONSTANT_COLOR,
                                    ONE_MINUS_CONSTANT_COLOR,
                                    CONSTANT_ALPHA,
                                    ONE_MINUS_CONSTANT_ALPHA,
                                    SRC_ALPHA_SATURATE,
                                    MAX);

DECLARE_ENUM_WITH_CONVERT_TO_STRING(EBlendOp,
                                    uint8_t,
                                    ADD,
                                    SUBTRACT,
                                    REVERSE_SUBTRACT,
                                    MIN_VALUE,
                                    MAX_VALUE,
                                    MAX, );

DECLARE_ENUM_WITH_CONVERT_TO_STRING(
    EFace, uint8_t, FRONT, BACK, FRONT_AND_BACK, MAX, );

DECLARE_ENUM_WITH_CONVERT_TO_STRING(EStencilOp,
                                    uint8_t,
                                    KEEP,
                                    ZERO,
                                    REPLACE,
                                    INCR,
                                    INCR_WRAP,
                                    DECR,
                                    DECR_WRAP,
                                    INVERT,
                                    MAX);

DECLARE_ENUM_WITH_CONVERT_TO_STRING(ECompareOp,
                                    uint8_t,
                                    NEVER,
                                    LESS,
                                    EQUAL,
                                    LEQUAL,
                                    GREATER,
                                    NOTEQUAL,
                                    GEQUAL,
                                    ALWAYS,
                                    MAX);

enum class ERenderBufferType : uint32_t {
  NONE    = 0,
  COLOR   = 0x00'00'00'01,
  DEPTH   = 0x00'00'00'02,
  STENCIL = 0x00'00'00'04,
  MAX     = 0xff'ff'ff'ff
};

DECLARE_ENUM_BIT_OPERATORS(ERenderBufferType)

DECLARE_ENUM_WITH_CONVERT_TO_STRING(EDrawBufferType,
                                    uint8_t,
                                    COLOR_ATTACHMENT0,
                                    COLOR_ATTACHMENT1,
                                    COLOR_ATTACHMENT2,
                                    COLOR_ATTACHMENT3,
                                    COLOR_ATTACHMENT4,
                                    COLOR_ATTACHMENT5,
                                    COLOR_ATTACHMENT6,
                                    COLOR_ATTACHMENT7,
                                    MAX);

DECLARE_ENUM_WITH_CONVERT_TO_STRING(EDepthBufferType,
                                    uint8_t,
                                    NONE,
                                    DEPTH16,
                                    DEPTH16_STENCIL8,
                                    DEPTH24,
                                    DEPTH24_STENCIL8,
                                    DEPTH32,
                                    DEPTH32_STENCIL8,
                                    MAX, );

DECLARE_ENUM_WITH_CONVERT_TO_STRING(ETextureAddressMode,
                                    uint8_t,
                                    REPEAT,
                                    MIRRORED_REPEAT,
                                    CLAMP_TO_EDGE,
                                    CLAMP_TO_BORDER,
                                    MIRROR_CLAMP_TO_EDGE,
                                    MAX, );

DECLARE_ENUM_WITH_CONVERT_TO_STRING(
    ETextureComparisonMode,
    uint8_t,
    NONE,
    COMPARE_REF_TO_TEXTURE,  // to use PCF filtering by using samplerXXShadow
                             // series.
    MAX, );

DECLARE_ENUM_WITH_CONVERT_TO_STRING(
    EPolygonMode, uint8_t, POINT, LINE, FILL, MAX);

DECLARE_ENUM_WITH_CONVERT_TO_STRING(EImageTextureAccessType,
                                    uint8_t,
                                    NONE,
                                    READ_ONLY,
                                    WRITE_ONLY,
                                    READ_WRITE,
                                    MAX, );

DECLARE_ENUM_WITH_CONVERT_TO_STRING(EFrontFace, uint8_t, CW, CCW, MAX);

DECLARE_ENUM_WITH_CONVERT_TO_STRING(
    ECullMode, uint8_t, NONE, BACK, FRONT, FRONT_AND_BACK, MAX);

enum class EMSAASamples : uint32_t {
  COUNT_1  = 0x00'00'00'01,
  COUNT_2  = 0x00'00'00'10,
  COUNT_4  = 0x00'00'01'00,
  COUNT_8  = 0x00'00'10'00,
  COUNT_16 = 0x00'01'00'00,
  COUNT_32 = 0x00'10'00'00,
  COUNT_64 = 0x01'00'00'00
};
DECLARE_ENUM_BIT_OPERATORS(EMSAASamples)

DECLARE_ENUM_WITH_CONVERT_TO_STRING(EAttachmentLoadStoreOp,
                                    uint8_t,
                                    LOAD_STORE,
                                    LOAD_DONTCARE,
                                    CLEAR_STORE,
                                    CLEAR_DONTCARE,
                                    DONTCARE_STORE,
                                    DONTCARE_DONTCARE,
                                    MAX);

enum class EShaderAccessStageFlag : uint32_t {
  VERTEX                  = 0x00'00'00'01,
  TESSELLATION_CONTROL    = 0x00'00'00'02,
  TESSELLATION_EVALUATION = 0x00'00'00'04,
  GEOMETRY                = 0x00'00'00'08,
  FRAGMENT                = 0x00'00'00'10,
  COMPUTE                 = 0x00'00'00'20,
  RAYTRACING              = 0x00'00'00'40,
  RAYTRACING_RAYGEN       = 0x00'00'00'80,
  RAYTRACING_MISS         = 0x00'00'01'00,
  RAYTRACING_CLOSESTHIT   = 0x00'00'02'00,
  RAYTRACING_ANYHIT       = 0x00'00'04'00,
  ALL_RAYTRACING = RAYTRACING_RAYGEN | RAYTRACING_MISS | RAYTRACING_CLOSESTHIT
                 | RAYTRACING_ANYHIT,
  ALL_GRAPHICS = 0x00'00'00'1F,
  ALL          = 0x7F'FF'FF'FF
};
DECLARE_ENUM_BIT_OPERATORS(EShaderAccessStageFlag)

DECLARE_ENUM_WITH_CONVERT_TO_STRING(EShaderBindingType,
                                    uint32_t,
                                    UNIFORMBUFFER,
                                    UNIFORMBUFFER_DYNAMIC,
                                    TEXTURE_SAMPLER_SRV,
                                    TEXTURE_SRV,
                                    TEXTURE_UAV,
                                    TEXTURE_ARRAY_SRV,
                                    SAMPLER,
                                    BUFFER_SRV,  // SSBO or StructuredBuffer
                                    BUFFER_UAV,
                                    BUFFER_UAV_DYNAMIC,
                                    BUFFER_TEXEL_SRV,
                                    BUFFER_TEXEL_UAV,
                                    ACCELERATION_STRUCTURE_SRV,
                                    SUBPASS_INPUT_ATTACHMENT,
                                    MAX);

enum class EVulkanBufferBits : uint32_t {
  TRANSFER_SRC                                 = 0x00'00'00'01,
  TRANSFER_DST                                 = 0x00'00'00'02,
  UNIFORM_TEXEL_BUFFER                         = 0x00'00'00'04,
  STORAGE_TEXEL_BUFFER                         = 0x00'00'00'08,
  UNIFORM_BUFFER                               = 0x00'00'00'10,
  STORAGE_BUFFER                               = 0x00'00'00'20,
  INDEX_BUFFER                                 = 0x00'00'00'40,
  VERTEX_BUFFER                                = 0x00'00'00'80,
  INDIRECT_BUFFER                              = 0x00'00'01'00,
  SHADER_DEVICE_ADDRESS                        = 0x00'02'00'00,
  TRANSFORM_FEEDBACK_BUFFER                    = 0x00'00'08'00,
  TRANSFORM_FEEDBACK_COUNTER_BUFFER            = 0x00'00'10'00,
  CONDITIONAL_RENDERING                        = 0x00'00'02'00,
  ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY = 0x00'08'00'00,
  ACCELERATION_STRUCTURE_STORAGE               = 0x00'10'00'00,
  SHADER_BINDING_TABLE                         = 0x00'00'04'00,
};
DECLARE_ENUM_BIT_OPERATORS(EVulkanBufferBits)

enum class EVulkanMemoryBits : uint32_t {
  DEVICE_LOCAL        = 0x00'00'00'01,
  HOST_VISIBLE        = 0x00'00'00'02,
  HOST_COHERENT       = 0x00'00'00'04,
  HOST_CACHED         = 0x00'00'00'08,
  LAZILY_ALLOCATED    = 0x00'00'00'10,
  PROTECTED           = 0x00'00'00'20,
  DEVICE_COHERENT_AMD = 0x00'00'00'40,
  DEVICE_UNCACHED_AMD = 0x00'00'00'80,
  RDMA_CAPABLE_NV     = 0x00'00'01'00,
  FLAG_BITS_MAX_ENUM  = 0x7F'FF'FF'FF
};
DECLARE_ENUM_BIT_OPERATORS(EVulkanMemoryBits)

enum class EColorMask : uint8_t {
  NONE = 0,
  R    = 0x01,
  G    = 0x02,
  B    = 0x04,
  A    = 0x08,
  ALL  = 0x0F
};
DECLARE_ENUM_BIT_OPERATORS(EColorMask)

DECLARE_ENUM_WITH_CONVERT_TO_STRING(EResourceLayout,
                                    uint8_t,
                                    UNDEFINED,
                                    GENERAL,
                                    UAV,
                                    COLOR_ATTACHMENT,
                                    DEPTH_STENCIL_ATTACHMENT,
                                    DEPTH_STENCIL_READ_ONLY,
                                    SHADER_READ_ONLY,
                                    TRANSFER_SRC,
                                    TRANSFER_DST,
                                    PREINITIALIZED,
                                    DEPTH_READ_ONLY_STENCIL_ATTACHMENT,
                                    DEPTH_ATTACHMENT_STENCIL_READ_ONLY,
                                    DEPTH_ATTACHMENT,
                                    DEPTH_READ_ONLY,
                                    STENCIL_ATTACHMENT,
                                    STENCIL_READ_ONLY,
                                    PRESENT_SRC,
                                    SHARED_PRESENT,
                                    SHADING_RATE_NV,
                                    FRAGMENT_DENSITY_MAP_EXT,
                                    READ_ONLY,
                                    ATTACHMENT,
                                    ACCELERATION_STRUCTURE,
                                    MAX);

enum class EPipelineStageMask : uint32_t {
  NONE                               = 0,
  TOP_OF_PIPE_BIT                    = 0x00'00'00'01,
  DRAW_INDIRECT_BIT                  = 0x00'00'00'02,
  VERTEX_INPUT_BIT                   = 0x00'00'00'04,
  VERTEX_SHADER_BIT                  = 0x00'00'00'08,
  TESSELLATION_CONTROL_SHADER_BIT    = 0x00'00'00'10,
  TESSELLATION_EVALUATION_SHADER_BIT = 0x00'00'00'20,
  GEOMETRY_SHADER_BIT                = 0x00'00'00'40,
  FRAGMENT_SHADER_BIT                = 0x00'00'00'80,
  EARLY_FRAGMENT_TESTS_BIT           = 0x00'00'01'00,
  LATE_FRAGMENT_TESTS_BIT            = 0x00'00'02'00,
  COLOR_ATTACHMENT_OUTPUT_BIT        = 0x00'00'04'00,
  COMPUTE_SHADER_BIT                 = 0x00'00'08'00,
  TRANSFER_BIT                       = 0x00'00'10'00,
  BOTTOM_OF_PIPE_BIT                 = 0x00'00'20'00,
  HOST_BIT                           = 0x00'00'40'00,
  ALL_GRAPHICS_BIT                   = 0x00'00'80'00,
  ALL_COMMANDS_BIT                   = 0x00'01'00'00,
};
DECLARE_ENUM_BIT_OPERATORS(EPipelineStageMask)

DECLARE_ENUM_WITH_CONVERT_TO_STRING(EPipelineDynamicState,
                                    uint8_t,
                                    VIEWPORT,
                                    SCISSOR,
                                    LINE_WIDTH,
                                    DEPTH_BIAS,
                                    BLEND_CONSTANTS,
                                    DEPTH_BOUNDS,
                                    STENCIL_COMPARE_MASK,
                                    STENCIL_WRITE_MASK,
                                    STENCIL_REFERENCE,
                                    CULL_MODE,
                                    FRONT_FACE,
                                    PRIMITIVE_TOPOLOGY,
                                    VIEWPORT_WITH_COUNT,
                                    SCISSOR_WITH_COUNT,
                                    VERTEX_INPUT_BINDING_STRIDE,
                                    DEPTH_TEST_ENABLE,
                                    DEPTH_WRITE_ENABLE,
                                    DEPTH_COMPARE_OP,
                                    DEPTH_BOUNDS_TEST_ENABLE,
                                    STENCIL_TEST_ENABLE,
                                    STENCIL_OP,
                                    RASTERIZER_DISCARD_ENABLE,
                                    DEPTH_BIAS_ENABLE,
                                    PRIMITIVE_RESTART_ENABLE,
                                    VIEWPORT_W_SCALING_NV,
                                    DISCARD_RECTANGLE_EXT,
                                    SAMPLE_LOCATIONS_EXT,
                                    RAY_TRACING_PIPELINE_STACK_SIZE_KHR,
                                    VIEWPORT_SHADING_RATE_PALETTE_NV,
                                    VIEWPORT_COARSE_SAMPLE_ORDER_NV,
                                    EXCLUSIVE_SCISSOR_NV,
                                    FRAGMENT_SHADING_RATE_KHR,
                                    LINE_STIPPLE_EXT,
                                    VERTEX_INPUT_EXT,
                                    PATCH_CONTROL_POINTS_EXT,
                                    LOGIC_OP_EXT,
                                    COLOR_WRITE_ENABLE_EXT,
                                    MAX);

DECLARE_ENUM_WITH_CONVERT_TO_STRING(
    EDescriptorHeapTypeDX12, uint8_t, CBV_SRV_UAV, SAMPLER, RTV, DSV, MAX);

DECLARE_ENUM_WITH_CONVERT_TO_STRING(
    ERTClearType, uint8_t, None, Color, DepthStencil, MAX);

enum class EBufferCreateFlag : uint32_t {
  NONE                            = 0,
  CPUAccess                       = 0x00'00'00'01,
  UAV                             = 0x00'00'00'02,
  Readback                        = 0x00'00'00'04,
  AccelerationStructureBuildInput = 0x00'00'00'08,
  VertexBuffer                    = 0x00'00'00'10,
  IndexBuffer                     = 0x00'00'00'20,
  IndirectCommand                 = 0x00'00'00'40,
  ShaderBindingTable              = 0x00'00'00'80,
  AccelerationStructure           = 0x00'00'01'00,
};
DECLARE_ENUM_BIT_OPERATORS(EBufferCreateFlag)

enum class ETextureCreateFlag : uint32_t {
  NONE         = 0,
  RTV          = 0x00'00'00'01,
  UAV          = 0x00'00'00'02,
  CPUAccess    = 0x00'00'00'04,
  TransferDst  = 0x00'00'00'08,
  TransferSrc  = 0x00'00'00'10,
  ShadingRate  = 0x00'00'00'20,
  DSV          = 0x00'00'00'40,
  SubpassInput = 0x00'00'00'40,
  Memoryless   = 0x00'00'00'80,
};
DECLARE_ENUM_BIT_OPERATORS(ETextureCreateFlag)

struct DepthStencilClearType {
  float    m_depth_;
  uint32_t m_stencil_;
};

class RTClearValue {
  public:
  static const RTClearValue s_kInvalid;

  union ClearValueType {
    float                 m_color_[4];
    DepthStencilClearType m_depthStencil_;
  };

  constexpr RTClearValue() = default;

  RTClearValue(const math::Vector4Df& InColor)
      : m_type_(ERTClearType::Color) {
    m_clearValue_.m_color_[0] = InColor.x();
    m_clearValue_.m_color_[1] = InColor.y();
    m_clearValue_.m_color_[2] = InColor.z();
    m_clearValue_.m_color_[3] = InColor.w();
  }

  constexpr RTClearValue(float InR, float InG, float InB, float InA)
      : m_type_(ERTClearType::Color) {
    m_clearValue_.m_color_[0] = InR;
    m_clearValue_.m_color_[1] = InG;
    m_clearValue_.m_color_[2] = InB;
    m_clearValue_.m_color_[3] = InA;
  }

  constexpr RTClearValue(float InDepth, uint32_t InStencil)
      : m_type_(ERTClearType::DepthStencil) {
    m_clearValue_.m_depthStencil_.m_depth_   = InDepth;
    m_clearValue_.m_depthStencil_.m_stencil_ = InStencil;
  }

  void SetColor(const math::Vector4Df& InColor) {
    m_type_                   = ERTClearType::Color;
    m_clearValue_.m_color_[0] = InColor.x();
    m_clearValue_.m_color_[1] = InColor.y();
    m_clearValue_.m_color_[2] = InColor.z();
    m_clearValue_.m_color_[3] = InColor.w();
  }

  void SetDepthStencil(float InDepth, uint8_t InStencil) {
    m_type_                                  = ERTClearType::DepthStencil;
    m_clearValue_.m_depthStencil_.m_depth_   = InDepth;
    m_clearValue_.m_depthStencil_.m_stencil_ = InStencil;
  }

  const float* GetCleraColor() const { return &m_clearValue_.m_color_[0]; }

  DepthStencilClearType GetCleraDepthStencil() const {
    return m_clearValue_.m_depthStencil_;
  }

  float GetCleraDepth() const { return m_clearValue_.m_depthStencil_.m_depth_; }

  uint32_t GetCleraStencil() const {
    return m_clearValue_.m_depthStencil_.m_stencil_;
  }

  ClearValueType GetClearValue() const { return m_clearValue_; }

  void ResetToNoneType() { m_type_ = ERTClearType::None; }

  ERTClearType GetType() const { return m_type_; }

  size_t GetHash() const {
    if (m_type_ == ERTClearType::Color) {
      return GETHASH_FROM_INSTANT_STRUCT(m_type_,
                                         m_clearValue_.m_color_[0],
                                         m_clearValue_.m_color_[1],
                                         m_clearValue_.m_color_[2],
                                         m_clearValue_.m_color_[3]);
    }

    return GETHASH_FROM_INSTANT_STRUCT(
        m_type_,
        m_clearValue_.m_depthStencil_.m_depth_,
        m_clearValue_.m_depthStencil_.m_stencil_);
  }

  private:
  ERTClearType   m_type_ = ERTClearType::None;
  ClearValueType m_clearValue_;
};

struct BaseVertex {
  math::Vector3Df m_position_  = math::g_zeroVector<float, 3>();
  math::Vector3Df m_normal_    = math::g_zeroVector<float, 3>();
  math::Vector3Df m_tangent_   = math::g_zeroVector<float, 3>();
  math::Vector3Df m_bitangent_ = math::g_zeroVector<float, 3>();
  math::Vector2Df m_texCoord_  = math::g_zeroVector<float, 2>();
};

struct PositionOnlyVertex {
  math::Vector3Df m_position_ = math::g_zeroVector<float, 3>();
};

}  // namespace game_engine

#endif  // GAME_ENGINE_RHI_TYPE_H