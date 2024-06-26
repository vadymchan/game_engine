
#include "gfx/rhi/vulkan/render_target_pool_vk.h"

#include "gfx/rhi/vulkan/render_target_vk.h"
#include "gfx/rhi/vulkan/rhi_vk.h"

namespace game_engine {

std::map<uint64_t, std::list<RenderTargetPoolVk::RenderTargetPoolResourceVk>>
    RenderTargetPoolVk::RenderTargetResourceMap;
std::map<RenderTargetVk*, uint64_t>
    RenderTargetPoolVk::RenderTargetHashVariableMap;

std::shared_ptr<RenderTargetVk> RenderTargetPoolVk::GetRenderTarget(
    const RenderTargetInfoVk& info) {
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

  auto renderTargetPtr = g_rhi_vk->CreateRenderTarget(info);
  if (renderTargetPtr) {
    renderTargetPtr->CreatedFromRenderTargetPool = true;
    RenderTargetResourceMap[hash].push_back({true, renderTargetPtr});
    RenderTargetHashVariableMap[renderTargetPtr.get()] = hash;
  }

  return renderTargetPtr;
}

void RenderTargetPoolVk::ReturnRenderTarget(RenderTargetVk* renderTarget) {
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

void RenderTargetPoolVk::Release() {
  RenderTargetResourceMap.clear();
  RenderTargetHashVariableMap.clear();
}

}  // namespace game_engine
