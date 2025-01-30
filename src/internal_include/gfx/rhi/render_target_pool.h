#ifndef GAME_ENGINE_RENDER_TARGET_POOL_H
#define GAME_ENGINE_RENDER_TARGET_POOL_H

#include "gfx/rhi/render_target.h"
#include "gfx/rhi/rhi_type.h"

#include <list>
#include <map>
#include <memory>

namespace game_engine {

class RenderTargetPool {
  public:
  // ======= BEGIN: public nested types =======================================

  struct RenderTargetPoolResource {
    bool                          m_isUsing_ = false;
    std::shared_ptr<RenderTarget> m_renderTarget_;
  };

  // ======= END: public nested types   =======================================

  // ======= BEGIN: public static methods =====================================

  static void s_seturnRenderTarget(RenderTarget* renderTarget);

  static void s_releaseForRecreateSwapchain() {
    m_renderTargetResourceMap_.clear();
    m_renderTargetHashVariableMap_.clear();
  }

  static void s_release() {
    m_renderTargetResourceMap_.clear();
    m_renderTargetHashVariableMap_.clear();
  }

  static std::shared_ptr<RenderTarget> s_getRenderTarget(
      const RenderTargetInfo& info);

  // ======= END: public static methods   =====================================

  // ======= BEGIN: public static fields ======================================

  static std::map<size_t, std::list<RenderTargetPoolResource> >
                                         m_renderTargetResourceMap_;
  static std::map<RenderTarget*, size_t> m_renderTargetHashVariableMap_;

  // ======= END: public static fields   ======================================

  // ======= BEGIN: public constructors =======================================

  RenderTargetPool();

  // ======= END: public constructors   =======================================

  // ======= BEGIN: public destructor =========================================

  ~RenderTargetPool();

  // ======= END: public destructor   =========================================
};

}  // namespace game_engine

#endif  // GAME_ENGINE_RENDER_TARGET_POOL_H
