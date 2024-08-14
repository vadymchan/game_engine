#ifndef GAME_ENGINE_FRAME_BUFFER_POOL_H
#define GAME_ENGINE_FRAME_BUFFER_POOL_H

#include "gfx/rhi/frame_buffer.h"
#include "gfx/rhi/rhi_type.h"

#include <list>
#include <map>
#include <memory>

namespace game_engine {

class FrameBufferPool {
  public:
  FrameBufferPool();
  ~FrameBufferPool();

  static std::shared_ptr<FrameBuffer> GetFrameBuffer(
      const FrameBufferInfo& info);
  static void ReturnFrameBuffer(FrameBuffer* renderTarget);

  static void Release() {
    FrameBufferResourceMap.clear();
    FrameBufferHashVariableMap.clear();
  }

  struct FrameBufferPoolResource {
    bool                          IsUsing = false;
    std::shared_ptr<FrameBuffer> FrameBufferPtr;
  };

  static std::map<size_t, std::list<FrameBufferPoolResource> >
                                         FrameBufferResourceMap;
  static std::map<FrameBuffer*, size_t> FrameBufferHashVariableMap;

  //static struct Texture* GetNullTexture(ETextureType type);
};

}  // namespace game_engine

#endif  // GAME_ENGINE_FRAME_BUFFER_POOL_H