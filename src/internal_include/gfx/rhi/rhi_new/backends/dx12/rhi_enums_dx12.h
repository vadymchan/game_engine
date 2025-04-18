#ifndef GAME_ENGINE_RHI_DX12_UTIL_H
#define GAME_ENGINE_RHI_DX12_UTIL_H

#include "gfx/rhi/rhi_new/common/rhi_enums.h"
#include "platform/windows/windows_platform_setup.h"

#ifdef GAME_ENGINE_RHI_DX12

#include <unordered_map>

namespace game_engine {
namespace gfx {
namespace rhi {

enum class DescriptorHeapTypeDx12 : uint8_t {
  CbvSrvUav,
  Sampler,
  Rtv,
  Dsv,
  Count
};

DXGI_FORMAT   g_getTextureFormatDx12(TextureFormat textureFormat);
TextureFormat g_getTextureFormatDx12(DXGI_FORMAT formatType);

int g_getTextureComponentCountDx12(TextureFormat type);
int g_getTexturePixelSizeDx12(TextureFormat type);

TextureType              g_getTextureDimensionDx12(D3D12_RESOURCE_DIMENSION type, bool isArray);
D3D12_RESOURCE_DIMENSION g_getTextureDimensionDx12(TextureType type);

D3D12_DESCRIPTOR_HEAP_TYPE g_getDescriptorHeapTypeDx12(DescriptorHeapTypeDx12 heapType);
DescriptorHeapTypeDx12     g_getDescriptorHeapTypeDx12(D3D12_DESCRIPTOR_HEAP_TYPE heapType);

D3D12_DESCRIPTOR_RANGE_TYPE g_getShaderBindingTypeDx12(ShaderBindingType bindingType);
ShaderBindingType           g_getShaderBindingTypeDx12(D3D12_DESCRIPTOR_RANGE_TYPE bindingType);

D3D12_TEXTURE_ADDRESS_MODE g_getTextureAddressModeDx12(TextureAddressMode addressMode);
TextureAddressMode         g_getTextureAddressModeDx12(D3D12_TEXTURE_ADDRESS_MODE addressMode);

D3D12_COMPARISON_FUNC g_getCompareOpDx12(CompareOp compareOp);
CompareOp             g_getCompareOpDx12(D3D12_COMPARISON_FUNC compareOp);

D3D12_STENCIL_OP g_getStencilOpDx12(StencilOp stencilOp);
StencilOp        g_getStencilOpDx12(D3D12_STENCIL_OP stencilOp);

D3D12_PRIMITIVE_TOPOLOGY_TYPE g_getPrimitiveTopologyTypeOnlyDx12(PrimitiveType type);
PrimitiveType                 g_getPrimitiveTopologyTypeOnlyDx12(D3D12_PRIMITIVE_TOPOLOGY_TYPE type);

D3D_PRIMITIVE_TOPOLOGY g_getPrimitiveTopologyDx12(PrimitiveType type);
PrimitiveType          g_getPrimitiveTopologyDx12(D3D_PRIMITIVE_TOPOLOGY type);

D3D12_INPUT_CLASSIFICATION g_getVertexInputRateDx12(VertexInputRate inputRate);
VertexInputRate            g_getVertexInputRateDx12(D3D12_INPUT_CLASSIFICATION inputRate);

D3D12_FILL_MODE g_getPolygonModeDx12(PolygonMode polygonMode);
PolygonMode     g_getPolygonModeDx12(D3D12_FILL_MODE polygonMode);

D3D12_CULL_MODE g_getCullModeDx12(CullMode cullMode);
CullMode        g_getCullModeDx12(D3D12_CULL_MODE cullMode);

D3D12_BLEND g_getBlendFactorDx12(BlendFactor type);
BlendFactor g_getBlendFactorDx12(D3D12_BLEND type);

D3D12_BLEND_OP g_getBlendOpDx12(BlendOp blendOp);
BlendOp        g_getBlendOpDx12(D3D12_BLEND_OP blendOp);

uint8_t g_getColorMaskDx12(ColorMask type);

D3D12_FILTER g_getTextureFilterDx12(TextureFilter minification, TextureFilter magnification, bool isComparison = false);

void g_getDepthFormatForSRV(DXGI_FORMAT& texFormat, DXGI_FORMAT& srvFormat, DXGI_FORMAT originalTexFormat);

D3D12_RESOURCE_STATES g_getResourceLayoutDx12(ResourceLayout resourceLayout);
ResourceLayout        g_getResourceLayoutDx12(D3D12_RESOURCE_STATES resourceState);

}  // namespace rhi
}  // namespace gfx
}  // namespace game_engine

#endif  // GAME_ENGINE_RHI_DX12
#endif  // GAME_ENGINE_RHI_DX12_UTIL_H