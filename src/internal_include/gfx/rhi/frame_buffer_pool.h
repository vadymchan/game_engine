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
  // ======= BEGIN: public nested types =======================================

  struct FrameBufferPoolResource {
    bool                         m_isUsing_ = false;
    std::shared_ptr<FrameBuffer> m_frameBufferPtr_;
  };

  // ======= END: public nested types   =======================================

  // ======= BEGIN: public static methods =====================================

  static void s_returnFrameBuffer(FrameBuffer* renderTarget);

  static void s_release() {
    s_frameBufferResourceMap.clear();
    s_frameBufferHashVariableMap.clear();
  }

  static std::shared_ptr<FrameBuffer> s_getFrameBuffer(
      const FrameBufferInfo& info);

  // ======= END: public static methods   =====================================

  // ======= BEGIN: public static fields ======================================

  static std::map<size_t, std::list<FrameBufferPoolResource> >
                                        s_frameBufferResourceMap;
  static std::map<FrameBuffer*, size_t> s_frameBufferHashVariableMap;

  // ======= END: public static fields   ======================================

  // ======= BEGIN: public constructors =======================================

  FrameBufferPool();

  // ======= END: public constructors   =======================================

  // ======= BEGIN: public destructor =========================================

  ~FrameBufferPool();

  // ======= END: public destructor   =========================================
};

}  // namespace game_engine

#endif  // GAME_ENGINE_FRAME_BUFFER_POOL_H
