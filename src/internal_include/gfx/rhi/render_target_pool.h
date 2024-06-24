#ifndef GAME_ENGINE_RENDER_TARGET_POOL_H
#define GAME_ENGINE_RENDER_TARGET_POOL_H

#include "gfx/rhi/rhi_type.h"
#include "gfx/rhi/render_target.h"

#include <list>
#include <map>
#include <memory>

namespace game_engine {
//extern std::shared_ptr<jRenderTarget> g_EyeAdaptationARTPtr;
//extern std::shared_ptr<jRenderTarget> g_EyeAdaptationBRTPtr;

class jRenderTargetPool {
  public:
  jRenderTargetPool();
  ~jRenderTargetPool();

  static std::shared_ptr<jRenderTarget> GetRenderTarget(
      const jRenderTargetInfo& info);
  static void ReturnRenderTarget(jRenderTarget* renderTarget);

  static void ReleaseForRecreateSwapchain() {
    RenderTargetResourceMap.clear();
    RenderTargetHashVariableMap.clear();
  }

  static void Release() {
    //if (g_EyeAdaptationARTPtr) {
    //  g_EyeAdaptationARTPtr->Return();
    //  g_EyeAdaptationARTPtr.reset();
    //}
    //if (g_EyeAdaptationBRTPtr) {
    //  g_EyeAdaptationBRTPtr->Return();
    //  g_EyeAdaptationBRTPtr.reset();
    //}

    RenderTargetResourceMap.clear();
    RenderTargetHashVariableMap.clear();
  }

  struct jRenderTargetPoolResource {
    bool                           IsUsing = false;
    std::shared_ptr<jRenderTarget> RenderTargetPtr;
  };

  static std::map<size_t, std::list<jRenderTargetPoolResource> >
                                          RenderTargetResourceMap;
  static std::map<jRenderTarget*, size_t> RenderTargetHashVariableMap;

  //static struct jTexture* GetNullTexture(ETextureType type);
};

}  // namespace game_engine

#endif  // GAME_ENGINE_RENDER_TARGET_POOL_H