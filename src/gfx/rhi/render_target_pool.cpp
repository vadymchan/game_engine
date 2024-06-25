#include "gfx/rhi/render_target_pool.h"
#include "gfx/rhi/rhi.h"

namespace game_engine {

// std::shared_ptr<jRenderTarget> g_EyeAdaptationARTPtr;
// std::shared_ptr<jRenderTarget> g_EyeAdaptationBRTPtr;

std::map<size_t, std::list<jRenderTargetPool::jRenderTargetPoolResource> >
                                 jRenderTargetPool::RenderTargetResourceMap;
std::map<jRenderTarget*, size_t> jRenderTargetPool::RenderTargetHashVariableMap;

// struct jTexture* jRenderTargetPool::GetNullTexture(ETextureType type) {
//   static std::shared_ptr<jRenderTarget> RTPtr
//       = jRenderTargetPool::GetRenderTarget(
//           {type, ETextureFormat::RGBA8, 2, 2, 1});
//   return RTPtr->GetTexture();
// }

jRenderTargetPool::jRenderTargetPool() {
}

jRenderTargetPool::~jRenderTargetPool() {
}

std::shared_ptr<jRenderTarget> jRenderTargetPool::GetRenderTarget(
    const jRenderTargetInfo& info) {
  auto hash = info.GetHash();

  auto it_find = RenderTargetResourceMap.find(hash);
  if (RenderTargetResourceMap.end() != it_find) {
    auto& renderTargets = it_find->second;
    for (auto& iter : renderTargets) {
      if (!iter.IsUsing) {
        iter.IsUsing = true;
        return iter.RenderTargetPtr;
      }
    }
  }

  auto renderTargetPtr = g_rhi->CreateRenderTarget(info);
  if (renderTargetPtr) {
    renderTargetPtr->bCreatedFromRenderTargetPool = true;
    RenderTargetResourceMap[hash].push_back({true, renderTargetPtr});
    RenderTargetHashVariableMap[renderTargetPtr.get()] = hash;
  }

  return renderTargetPtr;
}

void jRenderTargetPool::ReturnRenderTarget(jRenderTarget* renderTarget) {
  auto it_find = RenderTargetHashVariableMap.find(renderTarget);
  if (RenderTargetHashVariableMap.end() == it_find) {
    return;
  }

  const size_t hash = it_find->second;
  for (auto& iter : RenderTargetResourceMap[hash]) {
    if (renderTarget == iter.RenderTargetPtr.get()) {
      iter.IsUsing = false;
      break;
    }
  }
}

}  // namespace game_engine