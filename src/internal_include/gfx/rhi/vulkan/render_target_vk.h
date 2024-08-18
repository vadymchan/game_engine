// TODO: consider delete file
//#ifndef GAME_ENGINE_RENDER_TARGET_VK_H
//#define GAME_ENGINE_RENDER_TARGET_VK_H
//
//#include "gfx/rhi/instant_struct.h"
//#include "gfx/rhi/vulkan/rhi_type_vk.h"
//#include "gfx/rhi/vulkan/semaphore_vk.h"
//#include "gfx/rhi/vulkan/swapchain_vk.h"
//#include "gfx/rhi/vulkan/texture_vk.h"
//#include "gfx/rhi/vulkan/utils_vk.h"
//#include "utils/third_party/xxhash_util.h"
//
//#include <cassert>
//#include <memory>
//
//namespace game_engine {
//
//struct DepthStencilClearTypeVk {
//  float    Depth;
//  uint32_t Stencil;
//};
//
//class RTClearValueVk {
//  public:
//  static const RTClearValueVk Invalid;
//
//  union ClearValueTypeVk {
//    float                   Color[4];
//    DepthStencilClearTypeVk DepthStencil;
//  };
//
//  constexpr RTClearValueVk() = default;
//
//  RTClearValueVk(const math::Vector4Di& InColor)
//      : Type(ERTClearType::Color) {
//    ClearValue.Color[0] = InColor.x();
//    ClearValue.Color[1] = InColor.y();
//    ClearValue.Color[2] = InColor.z();
//    ClearValue.Color[3] = InColor.w();
//  }
//
//  constexpr RTClearValueVk(float InR, float InG, float InB, float InA)
//      : Type(ERTClearType::Color) {
//    ClearValue.Color[0] = InR;
//    ClearValue.Color[1] = InG;
//    ClearValue.Color[2] = InB;
//    ClearValue.Color[3] = InA;
//  }
//
//  constexpr RTClearValueVk(float InDepth, uint32_t InStencil)
//      : Type(ERTClearType::DepthStencil) {
//    ClearValue.DepthStencil.Depth   = InDepth;
//    ClearValue.DepthStencil.Stencil = InStencil;
//  }
//
//  void SetColor(const math::Vector4Di& InColor) {
//    Type                = ERTClearType::Color;
//    ClearValue.Color[0] = InColor.x();
//    ClearValue.Color[1] = InColor.y();
//    ClearValue.Color[2] = InColor.z();
//    ClearValue.Color[3] = InColor.w();
//  }
//
//  void SetDepthStencil(float InDepth, uint8_t InStencil) {
//    Type                            = ERTClearType::DepthStencil;
//    ClearValue.DepthStencil.Depth   = InDepth;
//    ClearValue.DepthStencil.Stencil = InStencil;
//  }
//
//  const float* GetCleraColor() const { return &ClearValue.Color[0]; }
//
//  DepthStencilClearTypeVk GetCleraDepthStencil() const {
//    return ClearValue.DepthStencil;
//  }
//
//  float GetCleraDepth() const { return ClearValue.DepthStencil.Depth; }
//
//  uint32_t GetCleraStencil() const { return ClearValue.DepthStencil.Stencil; }
//
//  ClearValueTypeVk GetClearValue() const { return ClearValue; }
//
//  void ResetToNoneType() { Type = ERTClearType::None; }
//
//  ERTClearType GetType() const { return Type; }
//
//  size_t GetHash() const {
//    if (Type == ERTClearType::Color) {
//      return GETHASH_FROM_INSTANT_STRUCT(Type,
//                                         ClearValue.Color[0],
//                                         ClearValue.Color[1],
//                                         ClearValue.Color[2],
//                                         ClearValue.Color[3]);
//    }
//
//    return GETHASH_FROM_INSTANT_STRUCT(
//        Type, ClearValue.DepthStencil.Depth, ClearValue.DepthStencil.Stencil);
//  }
//
//  bool operator==(const RTClearValueVk& InRHS) const {
//    if (Type == ERTClearType::Color) {
//      return ClearValue.Color[0] == InRHS.ClearValue.Color[0]
//          && ClearValue.Color[1] == InRHS.ClearValue.Color[1]
//          && ClearValue.Color[2] == InRHS.ClearValue.Color[2]
//          && ClearValue.Color[3] == InRHS.ClearValue.Color[3];
//    }
//
//    return ClearValue.DepthStencil.Depth == InRHS.ClearValue.DepthStencil.Depth
//        && ClearValue.DepthStencil.Stencil
//               == InRHS.ClearValue.DepthStencil.Stencil;
//  }
//
//  private:
//  ERTClearType     Type = ERTClearType::None;
//  ClearValueTypeVk ClearValue;
//};
//
//class RenderTargetInfoVk {
//  public:
//  size_t GetHash() const {
//    return GETHASH_FROM_INSTANT_STRUCT(Type,
//                                       Format,
//                                       Extent.width(),
//                                       Extent.height(),
//                                       LayerCount,
//                                       IsGenerateMipmap,
//                                       SampleCount,
//                                       m_rtClearValue.GetHash(),
//                                       TextureCreateFlag,
//                                       IsUseAsSubpassInput,
//                                       IsMemoryless);
//  }
//
//  ETextureType       Type                = ETextureType::TEXTURE_2D;
//  ETextureFormat     Format              = ETextureFormat::RGB8;
//  math::Dimension2Di Extent              = math::Dimension2Di(0);
//  uint32_t           LayerCount          = 1;
//  bool               IsGenerateMipmap    = false;
//  EMSAASamples       SampleCount         = EMSAASamples::COUNT_1;
//  bool               IsUseAsSubpassInput = false;
//  bool               IsMemoryless        = false;
//  RTClearValueVk     m_rtClearValue        = RTClearValueVk::Invalid;
//  ETextureCreateFlag TextureCreateFlag   = ETextureCreateFlag::RTV;
//};
//
//class RenderTargetVk {
//  public:
//  RenderTargetVk() = default;
//
//  RenderTargetVk(const std::shared_ptr<TextureVk>& InTexturePtr)
//      : m_TexturePtr_(InTexturePtr) {
//    if (InTexturePtr) {
//      Info.Type        = InTexturePtr->type;
//      Info.Format      = InTexturePtr->format;
//      Info.Extent      = InTexturePtr->extent;
//      Info.LayerCount  = InTexturePtr->layerCount;
//      Info.SampleCount = InTexturePtr->m_sampleCount_;
//    }
//  }
//
//  static std::shared_ptr<RenderTargetVk> CreateFromTexture(
//      const std::shared_ptr<TextureVk>& texturePtr) {
//    return std::make_shared<RenderTargetVk>(texturePtr);
//  }
//
//  TextureVk* GetTexture() const { return m_TexturePtr_.get(); }
//
//  const RenderTargetInfoVk& GetInfo() const { return Info; }
//
//  size_t GetHash() const {
//    if (Hash) {
//      return Hash;
//    }
//
//    Hash = Info.GetHash();
//    if (GetTexture()) {
//      Hash = XXH64(reinterpret_cast<uint64_t>(GetTexture()->image), Hash);
//    }
//    return Hash;
//  }
//
//  void Return();
//
//  mutable size_t             Hash = 0;
//  RenderTargetInfoVk         Info;
//  std::shared_ptr<TextureVk> m_TexturePtr_;
//  bool CreatedFromRenderTargetPool;  // TODO: consider whether is needed
//};
//
//
//// TODO: remove
//// ==================== SceneRenderTarget ( for Renderer) =====================
//
//// struct SceneRenderTarget {
////   std::shared_ptr<RenderTargetVk> ColorPtr;
////   std::shared_ptr<RenderTargetVk> DepthPtr;
////   std::shared_ptr<RenderTargetVk> ResolvePtr;
////
////   // Final rendered image, post-processed
////   std::shared_ptr<RenderTargetVk> FinalColorPtr;
////
////   void Create(std::shared_ptr<Window> window,
////               const SwapchainImageVk* InSwapchain);
////
////   void Return();
//// };
//
//}  // namespace game_engine
//
//#endif  // GAME_ENGINE_RENDER_TARGET_VK_H