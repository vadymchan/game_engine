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

  static std::shared_ptr<FrameBuffer> s_getFrameBuffer(
      const FrameBufferInfo& info);
  static void s_returnFrameBuffer(FrameBuffer* renderTarget);

  static void release() {
    s_frameBufferResourceMap.clear();
    s_frameBufferHashVariableMap.clear();
  }

  struct FrameBufferPoolResource {
    bool                         m_isUsing_ = false;
    std::shared_ptr<FrameBuffer> m_frameBufferPtr_;
  };

  static std::map<size_t, std::list<FrameBufferPoolResource> >
                                        s_frameBufferResourceMap;
  static std::map<FrameBuffer*, size_t> s_frameBufferHashVariableMap;

  // static struct Texture* GetNullTexture(ETextureType type);
};

}  // namespace game_engine

#endif  // GAME_ENGINE_FRAME_BUFFER_POOL_H