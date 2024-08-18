#ifndef GAME_ENGINE_RENDER_TARGET_POOL_H
#define GAME_ENGINE_RENDER_TARGET_POOL_H

#include "gfx/rhi/render_target.h"
#include "gfx/rhi/rhi_type.h"

#include <list>
#include <map>
#include <memory>

namespace game_engine {
// extern std::shared_ptr<RenderTarget> g_EyeAdaptationARTPtr;
// extern std::shared_ptr<RenderTarget> g_EyeAdaptationBRTPtr;

class RenderTargetPool {
  public:
  RenderTargetPool();
  ~RenderTargetPool();

  static std::shared_ptr<RenderTarget> GetRenderTarget(
      const RenderTargetInfo& info);
  static void ReturnRenderTarget(RenderTarget* renderTarget);

  static void ReleaseForRecreateSwapchain() {
    m_renderTargetResourceMap_.clear();
    m_renderTargetHashVariableMap_.clear();
  }

  static void Release() {
    // if (g_EyeAdaptationARTPtr) {
    //   g_EyeAdaptationARTPtr->Return();
    //   g_EyeAdaptationARTPtr.reset();
    // }
    // if (g_EyeAdaptationBRTPtr) {
    //   g_EyeAdaptationBRTPtr->Return();
    //   g_EyeAdaptationBRTPtr.reset();
    // }

    m_renderTargetResourceMap_.clear();
    m_renderTargetHashVariableMap_.clear();
  }

  struct RenderTargetPoolResource {
    bool                          m_isUsing_ = false;
    std::shared_ptr<RenderTarget> m_renderTargetPtr_;
  };

  static std::map<size_t, std::list<RenderTargetPoolResource> >
                                         m_renderTargetResourceMap_;
  static std::map<RenderTarget*, size_t> m_renderTargetHashVariableMap_;

  // static struct Texture* GetNullTexture(ETextureType type);
};

}  // namespace game_engine

#endif  // GAME_ENGINE_RENDER_TARGET_POOL_H