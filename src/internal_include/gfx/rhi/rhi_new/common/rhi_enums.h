#ifndef GAME_ENGINE_RHI_ENUMS_H
#define GAME_ENGINE_RHI_ENUMS_H

#include "utils/enum/enum_util.h"

#include <cstdint>

namespace game_engine {
namespace gfx {
namespace rhi {

enum class RenderingApi {
  Vulkan,
  Dx12,
  Count
};

// primitive rendering types
enum class PrimitiveType : uint8_t {
  Points,
  Lines,
  LinesAdjacency,
  LineStripAdjacency,
  Triangles,
  TriangleStrip,
  TrianglesAdjacency,
  TriangleStripAdjacency,
  Count
};

// Vertex input classification
enum class VertexInputRate : uint8_t {
  Vertex,
  Instance,
  Count
};

// Buffer update frequency
enum class BufferType : uint8_t {
  Static,
  Dynamic,
  Count
};

enum class BufferElementType : uint8_t {
  Byte,
  ByteUnorm,
  Uint16,
  Uint32,
  Float,
  Count
};

enum class TextureFilterTarget : uint8_t {
  Minification = 0,
  Magnification,
  Count
};

enum class TextureFilter : uint8_t {
  Nearest,
  Linear,
  NearestMipmapNearest,
  LinearMipmapNearest,
  NearestMipmapLinear,
  LinearMipmapLinear,
  Count
};

enum class TextureType : uint8_t {
  Texture1D,
  Texture2D,
  Texture2DArray,
  Texture3D,
  Texture3DArray,
  TextureCube,
  Count
};

enum class TextureFormat : uint8_t {
  // Color formats
  Rgb8,
  Rgb16f,
  Rgb32f,
  R11g11b10f,

  Rgba8,
  Rgba16f,
  Rgba32f,
  Rgba8si,
  Rgba8ui,

  Bgra8,

  R8,
  R16f,
  R32f,
  R8ui,
  R32ui,

  Rg8,
  Rg16f,
  Rg32f,

  // Depth formats
  D16,
  D16S8,
  D24,
  D24S8,
  D32,
  D32S8,

  // Compressed formats
  Bc1Unorm,
  Bc2Unorm,
  Bc3Unorm,
  Bc4Unorm,
  Bc4Snorm,
  Bc5Unorm,
  Bc5Snorm,
  Bc6hUf16,
  Bc6hSf16,
  Bc7Unorm,
  Count
};

inline bool g_isDepthFormat(TextureFormat format) {
  switch (format) {
    case TextureFormat::D16:
    case TextureFormat::D16S8:
    case TextureFormat::D24:
    case TextureFormat::D24S8:
    case TextureFormat::D32:
    case TextureFormat::D32S8:
      return true;
    default:
      return false;
  }
}

inline bool g_isDepthOnlyFormat(TextureFormat format) {
  switch (format) {
    case TextureFormat::D16:
    case TextureFormat::D24:
    case TextureFormat::D32:
      return true;
    default:
      return false;
  }
}

enum class FormatType : uint8_t {
  Byte,
  UnsignedByte,
  ShortInt,
  Int,
  UnsignedInt,
  Half,
  Float,
  Count
};

// for color blending
enum class BlendFactor : uint8_t {
  Zero,
  One,
  SrcColor,
  OneMinusSrcColor,
  DstColor,
  OneMinusDstColor,
  SrcAlpha,
  OneMinusSrcAlpha,
  DstAlpha,
  OneMinusDstAlpha,
  ConstantColor,
  OneMinusConstantColor,
  ConstantAlpha,
  OneMinusConstantAlpha,
  SrcAlphaSaturate,
  Count
};

enum class BlendOp : uint8_t {
  Add,
  Subtract,
  ReverseSubtract,
  MinValue,
  MaxValue,
  Count
};

enum class Face : uint8_t {
  Front,
  Back,
  FrontAndBack,
  Count
};

enum class StencilOp : uint8_t {
  Keep,
  Zero,
  Replace,
  Incr,
  IncrWrap,
  Decr,
  DecrWrap,
  Invert,
  Count
};

enum class CompareOp : uint8_t {
  Never,
  Less,
  Equal,
  Lequal,
  Greater,
  Notequal,
  Gequal,
  Always,
  Count
};

// TODO: if there's a need, add bitwise operations to this enum
enum class RenderBufferType : uint32_t {
  None    = 0,
  Color   = 0x00'00'00'01,
  Depth   = 0x00'00'00'02,
  Stencil = 0x00'00'00'04,
  All     = 0xFF'FF'FF'FF
};

enum class DrawBufferType : uint8_t {
  ColorAttachment0,
  ColorAttachment1,
  ColorAttachment2,
  ColorAttachment3,
  ColorAttachment4,
  ColorAttachment5,
  ColorAttachment6,
  ColorAttachment7,
  Count
};

enum class DepthBufferType : uint8_t {
  None,
  Depth16,
  Depth16Stencil8,
  Depth24,
  Depth24Stencil8,
  Depth32,
  Depth32Stencil8,
  Count
};

enum class TextureAddressMode : uint8_t {
  Repeat,
  MirroredRepeat,
  ClampToEdge,
  ClampToBorder,
  MirrorClampToEdge,
  Count
};

enum class PolygonMode : uint8_t {
  Point,
  Line,
  Fill,
  Count
};

enum class ImageTextureAccessType : uint8_t {
  None,
  ReadOnly,
  WriteOnly,
  ReadWrite,
  Count
};

enum class FrontFace : uint8_t {
  Cw,   // Clockwise
  Ccw,  // Counter-clockwise
  Count
};

enum class CullMode : uint8_t {
  None,
  Back,
  Front,
  FrontAndBack,
  Count
};

enum class MSAASamples : uint32_t {
  Count1  = 0x00'00'00'01,
  Count2  = 0x00'00'00'10,
  Count4  = 0x00'00'01'00,
  Count8  = 0x00'00'10'00,
  Count16 = 0x00'01'00'00,
  Count32 = 0x00'10'00'00,
  Count64 = 0x01'00'00'00
};

enum class AttachmentLoadStoreOp : uint8_t {
  LoadStore,
  LoadDontcare,
  ClearStore,
  ClearDontcare,
  DontcareStore,
  DontcareDontcare,
  Count
};

// TODO: if there's a need, add bitwise operations to this enum
enum class ShaderStageFlag : uint32_t {
  Vertex                 = 0x00'00'00'01,
  TessellationControl    = 0x00'00'00'02,
  TessellationEvaluation = 0x00'00'00'04,
  Geometry               = 0x00'00'00'08,
  Fragment               = 0x00'00'00'10,
  Compute                = 0x00'00'00'20,
  Raytracing             = 0x00'00'00'40,
  RaytracingRaygen       = 0x00'00'00'80,
  RaytracingMiss         = 0x00'00'01'00,
  RaytracingClosesthit   = 0x00'00'02'00,
  RaytracingAnyhit       = 0x00'00'04'00,
  AllRaytracing          = RaytracingRaygen | RaytracingMiss | RaytracingClosesthit | RaytracingAnyhit,
  AllGraphics            = 0x00'00'00'1F,
  All                    = 0x7F'FF'FF'FF
};

enum class ShaderBindingType : uint32_t {
  Uniformbuffer,
  UniformbufferDynamic,
  TextureSamplerSrv,
  TextureSrv,
  TextureUav,
  TextureArraySrv,
  Sampler,
  BufferSrv,  // SSBO or StructuredBuffer
  BufferUav,
  BufferUavDynamic,
  BufferTexelSrv,
  BufferTexelUav,
  AccelerationStructureSrv,
  SubpassInputAttachment,
  Count
};

enum class ColorMask : uint8_t {
  None = 0,
  R    = 0x01,
  G    = 0x02,
  B    = 0x04,
  A    = 0x08,
  All  = 0x0F
};

DECLARE_ENUM_BIT_OPERATORS(ColorMask)

// Resource layout states
enum class ResourceLayout : uint8_t {
  Undefined,
  General,
  Uav,
  ColorAttachment,
  DepthStencilAttachment,
  DepthStencilReadOnly,
  ShaderReadOnly,
  TransferSrc,
  TransferDst,
  Preinitialized,
  DepthReadOnlyStencilAttachment,
  DepthAttachmentStencilReadOnly,
  DepthAttachment,
  DepthReadOnly,
  StencilAttachment,
  StencilReadOnly,
  PresentSrc,
  SharedPresent,
  ShadingRateNv,
  FragmentDensityMapExt,
  ReadOnly,
  Attachment,
  AccelerationStructure,
  Count
};

enum class PipelineType {
  Graphics,
  Compute
};

enum class PipelineStageMask : uint32_t {
  None                            = 0,
  TopOfPipeBit                    = 0x00'00'00'01,
  DrawIndirectBit                 = 0x00'00'00'02,
  VertexInputBit                  = 0x00'00'00'04,
  VertexShaderBit                 = 0x00'00'00'08,
  TessellationControlShaderBit    = 0x00'00'00'10,
  TessellationEvaluationShaderBit = 0x00'00'00'20,
  GeometryShaderBit               = 0x00'00'00'40,
  FragmentShaderBit               = 0x00'00'00'80,
  EarlyFragmentTestsBit           = 0x00'00'01'00,
  LateFragmentTestsBit            = 0x00'00'02'00,
  ColorAttachmentOutputBit        = 0x00'00'04'00,
  ComputeShaderBit                = 0x00'00'08'00,
  TransferBit                     = 0x00'00'10'00,
  BottomOfPipeBit                 = 0x00'00'20'00,
  HostBit                         = 0x00'00'40'00,
  AllGraphicsBit                  = 0x00'00'80'00,
  AllCommandsBit                  = 0x00'01'00'00,
};

enum class RTClearType : uint8_t {
  None,
  Color,
  DepthStencil,
  Count
};

enum class BufferCreateFlag : uint32_t {
  None                            = 0,
  CpuAccess                       = 0x00'00'00'01,
  Uav                             = 0x00'00'00'02,
  Readback                        = 0x00'00'00'04,
  AccelerationStructureBuildInput = 0x00'00'00'08,
  VertexBuffer                    = 0x00'00'00'10,
  IndexBuffer                     = 0x00'00'00'20,
  IndirectCommand                 = 0x00'00'00'40,
  ShaderBindingTable              = 0x00'00'00'80,
  AccelerationStructure           = 0x00'00'01'00,
};

DECLARE_ENUM_BIT_OPERATORS(BufferCreateFlag)

enum class TextureCreateFlag : uint32_t {
  None         = 0,
  Rtv          = 0x00'00'00'01,
  Uav          = 0x00'00'00'02,
  CpuAccess    = 0x00'00'00'04,
  TransferDst  = 0x00'00'00'08,
  TransferSrc  = 0x00'00'00'10,
  ShadingRate  = 0x00'00'00'20,
  Dsv          = 0x00'00'00'40,
  SubpassInput = 0x00'00'00'40,
  Memoryless   = 0x00'00'00'80,
};

DECLARE_ENUM_BIT_OPERATORS(TextureCreateFlag)

}  // namespace rhi
}  // namespace gfx
}  // namespace game_engine

#endif  // GAME_ENGINE_RHI_ENUMS_H