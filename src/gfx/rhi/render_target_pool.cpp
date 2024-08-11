#include "gfx/rhi/render_target_pool.h"
#include "gfx/rhi/rhi.h"

namespace game_engine {

// std::shared_ptr<RenderTarget> g_EyeAdaptationARTPtr;
// std::shared_ptr<RenderTarget> g_EyeAdaptationBRTPtr;

std::map<size_t, std::list<RenderTargetPool::RenderTargetPoolResource> >
                                 RenderTargetPool::RenderTargetResourceMap;
std::map<RenderTarget*, size_t> RenderTargetPool::RenderTargetHashVariableMap;

// struct Texture* RenderTargetPool::GetNullTexture(ETextureType type) {
//   static std::shared_ptr<RenderTarget> RTPtr
//       = RenderTargetPool::GetRenderTarget(
//           {type, ETextureFormat::RGBA8, 2, 2, 1});
//   return RTPtr->GetTexture();
// }

RenderTargetPool::RenderTargetPool() {
}

RenderTargetPool::~RenderTargetPool() {
}

std::shared_ptr<RenderTarget> RenderTargetPool::GetRenderTarget(
    const RenderTargetInfo& info) {
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

void RenderTargetPool::ReturnRenderTarget(RenderTarget* renderTarget) {
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