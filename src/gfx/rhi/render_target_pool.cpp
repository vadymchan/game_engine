#include "gfx/rhi/render_target_pool.h"

#include "gfx/rhi/rhi.h"

namespace game_engine {

std::map<size_t, std::list<RenderTargetPool::RenderTargetPoolResource> >
    RenderTargetPool::m_renderTargetResourceMap_;
std::map<RenderTarget*, size_t>
    RenderTargetPool::m_renderTargetHashVariableMap_;

RenderTargetPool::RenderTargetPool() {
}

RenderTargetPool::~RenderTargetPool() {
}

std::shared_ptr<RenderTarget> RenderTargetPool::s_getRenderTarget(
    const RenderTargetInfo& info) {
  auto hash = info.getHash();

  auto it_find = m_renderTargetResourceMap_.find(hash);
  if (m_renderTargetResourceMap_.end() != it_find) {
    auto& renderTargets = it_find->second;
    for (auto& iter : renderTargets) {
      if (!iter.m_isUsing_) {
        iter.m_isUsing_ = true;
        return iter.m_renderTarget_;
      }
    }
  }

  auto renderTargetPtr = g_rhi->createRenderTarget(info);
  if (renderTargetPtr) {
    renderTargetPtr->m_isCreatedFromRenderTargetPool_ = true;
    m_renderTargetResourceMap_[hash].push_back({true, renderTargetPtr});
    m_renderTargetHashVariableMap_[renderTargetPtr.get()] = hash;
  }

  return renderTargetPtr;
}

void RenderTargetPool::s_seturnRenderTarget(RenderTarget* renderTarget) {
  auto it_find = m_renderTargetHashVariableMap_.find(renderTarget);
  if (m_renderTargetHashVariableMap_.end() == it_find) {
    return;
  }

  const size_t hash = it_find->second;
  for (auto& iter : m_renderTargetResourceMap_[hash]) {
    if (renderTarget == iter.m_renderTarget_.get()) {
      iter.m_isUsing_ = false;
      break;
    }
  }
}

}  // namespace game_engine
