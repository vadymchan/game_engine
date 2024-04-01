//#include "gfx/rhi/vulkan/rhi_vk.h"
//
//namespace game_engine {
//
//template <ETextureFilter         TMinification,
//          ETextureFilter         TMagnification,
//          ETextureAddressMode    TAddressU,
//          ETextureAddressMode    TAddressV,
//          ETextureAddressMode    TAddressW,
//          float                  TMipLODBias,
//          float                  TMaxAnisotropy,
//          bool                   TIsEnableComparisonMode,
//          ECompareOp             TComparisonFunc,
//          float                  TMinLOD,
//          float                  TMaxLOD,
//          ETextureComparisonMode TTextureComparisonMode>
//SamplerStateInfoVk* TSamplerStateInfoVk<
//    TMinification,
//    TMagnification,
//    TAddressU,
//    TAddressV,
//    TAddressW,
//    TMipLODBias,
//    TMaxAnisotropy,
//    TIsEnableComparisonMode,
//    TComparisonFunc,
//    TMinLOD,
//    TMaxLOD,
//    TTextureComparisonMode>::Create(math::Vector4Df BorderColor) {
//  static SamplerStateInfoVk* CachedInfo = nullptr;
//  if (CachedInfo) {
//    return CachedInfo;
//  }
//
//  SamplerStateInfoVk initializer;
//  initializer.Minification           = TMinification;
//  initializer.Magnification          = TMagnification;
//  initializer.AddressU               = TAddressU;
//  initializer.AddressV               = TAddressV;
//  initializer.AddressW               = TAddressW;
//  initializer.MipLODBias             = TMipLODBias;
//  initializer.MaxAnisotropy          = TMaxAnisotropy;
//  initializer.IsEnableComparisonMode = TIsEnableComparisonMode;
//  initializer.TextureComparisonMode  = TTextureComparisonMode;
//  initializer.ComparisonFunc         = TComparisonFunc;
//  initializer.BorderColor            = BorderColor;
//  initializer.MinLOD                 = TMinLOD;
//  initializer.MaxLOD                 = TMaxLOD;
//  initializer.GetHash();
//  CachedInfo = g_rhi_vk->CreateSamplerState(initializer);
//  return CachedInfo;
//}
//
//template <EPolygonMode TPolygonMode,
//          ECullMode    TCullMode,
//          EFrontFace   TFrontFace,
//          bool         TDepthBiasEnable,
//          float        TDepthBiasConstantFactor,
//          float        TDepthBiasClamp,
//          float        TDepthBiasSlopeFactor,
//          float        TLineWidth,
//          bool         TDepthClampEnable,
//          bool         TRasterizerDiscardEnable,
//          EMSAASamples TSampleCount,
//          bool         TSampleShadingEnable,
//          float        TMinSampleShading,
//          bool         TAlphaToCoverageEnable,
//          bool         TAlphaToOneEnable>
//RasterizationStateInfoVk* TRasterizationStateInfoVk<
//    TPolygonMode,
//    TCullMode,
//    TFrontFace,
//    TDepthBiasEnable,
//    TDepthBiasConstantFactor,
//    TDepthBiasClamp,
//    TDepthBiasSlopeFactor,
//    TLineWidth,
//    TDepthClampEnable,
//    TRasterizerDiscardEnable,
//    TSampleCount,
//    TSampleShadingEnable,
//    TMinSampleShading,
//    TAlphaToCoverageEnable,
//    TAlphaToOneEnable>::Create(std::optional<EMSAASamples> sampleCountOpt) {
//  static RasterizationStateInfoVk* CachedInfo = nullptr;
//  if (CachedInfo) {
//    return CachedInfo;
//  }
//
//  RasterizationStateInfoVk initializer;
//  initializer.PolygonMode             = TPolygonMode;
//  initializer.CullMode                = TCullMode;
//  initializer.FrontFace               = TFrontFace;
//  initializer.DepthBiasEnable         = TDepthBiasEnable;
//  initializer.DepthBiasConstantFactor = TDepthBiasConstantFactor;
//  initializer.DepthBiasClamp          = TDepthBiasClamp;
//  initializer.DepthBiasSlopeFactor    = TDepthBiasSlopeFactor;
//  initializer.LineWidth               = TLineWidth;
//  initializer.DepthClampEnable        = TDepthClampEnable;
//  initializer.RasterizerDiscardEnable = TRasterizerDiscardEnable;
//
//  initializer.SampleCount           = sampleCountOpt.value_or(TSampleCount);
//  initializer.SampleShadingEnable   = TSampleShadingEnable;
//  initializer.MinSampleShading      = TMinSampleShading;
//  initializer.AlphaToCoverageEnable = TAlphaToCoverageEnable;
//  initializer.AlphaToOneEnable      = TAlphaToOneEnable;
//
//  initializer.GetHash();
//  // TODO: problem (should be in cpp)
//  CachedInfo = g_rhi_vk->CreateRasterizationState(initializer);
//  return CachedInfo;
//}
//
//template <bool       TDepthTestEnable,
//          bool       TDepthWriteEnable,
//          ECompareOp TDepthCompareOp,
//          bool       TDepthBoundsTestEnable,
//          bool       TStencilTestEnable,
//          float      TMinDepthBounds,
//          float      TMaxDepthBounds>
//DepthStencilStateInfoVk* TDepthStencilStateInfo<
//    TDepthTestEnable,
//    TDepthWriteEnable,
//    TDepthCompareOp,
//    TDepthBoundsTestEnable,
//    TStencilTestEnable,
//    TMinDepthBounds,
//    TMaxDepthBounds>::Create(StencilOpStateInfoVk* Front,
//                             StencilOpStateInfoVk* Back) {
//  static DepthStencilStateInfoVk* CachedInfo = nullptr;
//  if (CachedInfo) {
//    return CachedInfo;
//  }
//
//  DepthStencilStateInfoVk initializer;
//  initializer.DepthTestEnable       = TDepthTestEnable;
//  initializer.DepthWriteEnable      = TDepthWriteEnable;
//  initializer.DepthCompareOp        = TDepthCompareOp;
//  initializer.DepthBoundsTestEnable = TDepthBoundsTestEnable;
//  initializer.StencilTestEnable     = TStencilTestEnable;
//  initializer.Front                 = Front;
//  initializer.Back                  = Back;
//  initializer.MinDepthBounds        = TMinDepthBounds;
//  initializer.MaxDepthBounds        = TMaxDepthBounds;
//  initializer.GetHash();
//  CachedInfo = g_rhi_vk->CreateDepthStencilState(initializer);
//  return CachedInfo;
//}
//
//template <bool         TBlendEnable,
//          EBlendFactor TSrc,
//          EBlendFactor TDest,
//          EBlendOp     TBlendOp,
//          EBlendFactor TSrcAlpha,
//          EBlendFactor TDestAlpha,
//          EBlendOp     TAlphaBlendOp,
//          EColorMask   TColorWriteMask>
//BlendingStateInfoVk* TBlendingStateInfo<TBlendEnable,
//                                        TSrc,
//                                        TDest,
//                                        TBlendOp,
//                                        TSrcAlpha,
//                                        TDestAlpha,
//                                        TAlphaBlendOp,
//                                        TColorWriteMask>::Create() {
//  static BlendingStateInfoVk* CachedInfo = nullptr;
//  if (CachedInfo) {
//    return CachedInfo;
//  }
//
//  BlendingStateInfoVk initializer;
//  initializer.BlendEnable    = TBlendEnable;
//  initializer.Src            = TSrc;
//  initializer.Dest           = TDest;
//  initializer.BlendOp        = TBlendOp;
//  initializer.SrcAlpha       = TSrcAlpha;
//  initializer.DestAlpha      = TDestAlpha;
//  initializer.AlphaBlendOp   = TAlphaBlendOp;
//  initializer.ColorWriteMask = TColorWriteMask;
//  initializer.GetHash();
//  CachedInfo = g_rhi_vk->CreateBlendingState(initializer);
//  return CachedInfo;
//}
//
//}  // namespace game_engine