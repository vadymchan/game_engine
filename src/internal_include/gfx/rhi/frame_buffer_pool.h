#ifndef GAME_ENGINE_FRAME_BUFFER_POOL_H
#define GAME_ENGINE_FRAME_BUFFER_POOL_H

#include "gfx/rhi/frame_buffer.h"
#include "gfx/rhi/rhi_type.h"

#include <list>
#include <map>
#include <memory>

namespace game_engine {

class jFrameBufferPool {
  public:
  jFrameBufferPool();
  ~jFrameBufferPool();

  static std::shared_ptr<jFrameBuffer> GetFrameBuffer(
      const jFrameBufferInfo& info);
  static void ReturnFrameBuffer(jFrameBuffer* renderTarget);

  static void Release() {
    FrameBufferResourceMap.clear();
    FrameBufferHashVariableMap.clear();
  }

  struct jFrameBufferPoolResource {
    bool                          IsUsing = false;
    std::shared_ptr<jFrameBuffer> FrameBufferPtr;
  };

  static std::map<size_t, std::list<jFrameBufferPoolResource> >
                                         FrameBufferResourceMap;
  static std::map<jFrameBuffer*, size_t> FrameBufferHashVariableMap;

  //static struct jTexture* GetNullTexture(ETextureType type);
};

}  // namespace game_engine

#endif  // GAME_ENGINE_FRAME_BUFFER_POOL_H