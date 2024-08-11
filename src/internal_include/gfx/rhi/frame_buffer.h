#ifndef GAME_ENGINE_FRAME_BUFFER_H
#define GAME_ENGINE_FRAME_BUFFER_H

#include "gfx/rhi/instant_struct.h"
#include "gfx/rhi/rhi_type.h"
#include "gfx/rhi/texture.h"

#include <math_library/dimension.h>

#include <cstdint>
#include <memory>
#include <vector>

namespace game_engine {

struct FrameBufferInfo {

  size_t GetHash() const {
    return GETHASH_FROM_INSTANT_STRUCT(TextureType,
                                       Format,
                                       Extent.width(),
                                       Extent.height(),
                                       LayerCount,
                                       IsGenerateMipmap,
                                       SampleCount);
  }

  ETextureType       TextureType      = ETextureType::TEXTURE_2D;
  ETextureFormat     Format           = ETextureFormat::RGB8;
  math::Dimension2Di Extent           = math::Dimension2Di(0);
  int32_t            LayerCount       = 1;
  bool               IsGenerateMipmap = false;
  int32_t            SampleCount      = 1;
};

struct FrameBuffer : public std::enable_shared_from_this<FrameBuffer> {
  virtual ~FrameBuffer() {}

  // No need for now
  //virtual Texture* GetTexture(int32_t index = 0) const {
  //  return Textures[index].get();
  //}

  //virtual Texture* GetTextureDepth(int32_t index = 0) const {
  //  return TextureDepth.get();
  //}

  //virtual ETextureType GetTextureType() const { return Info.TextureType; }

  //virtual bool SetDepthAttachment(const std::shared_ptr<Texture>& InDepth) {
  //  TextureDepth = InDepth;
  //  return true;
  //}

  //virtual void SetDepthMipLevel(int32_t InLevel) {}

  //virtual bool FBOBegin(int index = 0, bool mrt = false) const { return true; }

  //virtual void End() const {}

  FrameBufferInfo                        Info;
  std::vector<std::shared_ptr<Texture> > Textures;
  std::shared_ptr<Texture>               TextureDepth;
};
}  // namespace game_engine

#endif  // GAME_ENGINE_FRAME_BUFFER_H