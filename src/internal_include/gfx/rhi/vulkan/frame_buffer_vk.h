// TODO: consider delete file
//#ifndef GAME_ENGINE_FRAME_BUFFER_VK_H
//#define GAME_ENGINE_FRAME_BUFFER_VK_H
//
//#include "gfx/rhi/instant_struct.h"
//#include "gfx/rhi/vulkan/texture_vk.h"
//
//#include <math_library/dimension.h>
//#include <vulkan/vulkan_core.h>
//
//#include <cstdint>
//#include <memory>
//#include <vector>
//
//namespace game_engine {
//
//struct FrameBufferInfoVk {
//  size_t GetHash() const {
//    return GETHASH_FROM_INSTANT_STRUCT(TextureType,
//                                       Format,
//                                       Extent.width(),
//                                       Extent.height(),
//                                       LayerCount,
//                                       IsGenerateMipmap,
//                                       SampleCount);
//  }
//
//  ETextureType       TextureType      = ETextureType::TEXTURE_2D;
//  ETextureFormat     Format           = ETextureFormat::RGB8;
//  math::Dimension2Di Extent           = math::Dimension2Di(0);
//  int32_t            LayerCount       = 1;
//  bool               IsGenerateMipmap = false;
//  int32_t            SampleCount      = 1;
//};
//
//struct FrameBufferVk {
//  virtual ~FrameBufferVk() {}
//
//  // No need for now
//  //
//  // virtual TextureVk* GetTexture(int32_t index = 0) const {
//  //  return Textures[index].get();
//  //}
//  //
//  // virtual TextureVk* GetTextureDepth(int32_t index = 0) const {
//  //   return TextureDepth.get();
//  // }
//  //
//  // virtual VkImageType GetTextureType() const { return Info.ImageType; }
//  //
//  // virtual bool SetDepthAttachment(const std::shared_ptr<TextureVk>& InDepth)
//  // {
//  //   TextureDepth = InDepth;
//  //   return true;
//  // }
//  //
//  // virtual void SetDepthMipLevel(int32_t InLevel) {}
//  //
//  // virtual bool FBOBegin(int index = 0, bool mrt = false) const { return true;
//  // }
//
//  // virtual void End() const {}
//
//  FrameBufferInfoVk                        Info;
//  std::vector<std::shared_ptr<TextureVk> > Textures;
//  std::shared_ptr<TextureVk>               TextureDepth;
//};
//
//}  // namespace game_engine
//
//#endif  // GAME_ENGINE_FRAME_BUFFER_VK_H