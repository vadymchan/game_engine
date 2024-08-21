#include "gfx/rhi/frame_buffer_pool.h"

#include "gfx/rhi/rhi.h"

namespace game_engine {

std::map<size_t, std::list<FrameBufferPool::FrameBufferPoolResource> >
                                FrameBufferPool::s_frameBufferResourceMap;
std::map<FrameBuffer*, size_t> FrameBufferPool::s_frameBufferHashVariableMap;

//struct Texture* FrameBufferPool::GetNullTexture(ETextureType type) {
//  switch (type) {
//    case ETextureType::TEXTURE_2D: {
//      static auto temp = FrameBufferPool::s_getFrameBuffer(
//          {ETextureType::TEXTURE_2D, ETextureFormat::RGBA8, 2, 2, 1});
//      return temp->getTexture();
//    }
//    case ETextureType::TEXTURE_2D_ARRAY: {
//      static auto temp = FrameBufferPool::s_getFrameBuffer(
//          {ETextureType::TEXTURE_2D_ARRAY, ETextureFormat::RGBA8, 2, 2, 1});
//      return temp->getTexture();
//    }
//    case ETextureType::TEXTURE_CUBE: {
//      static auto temp = FrameBufferPool::s_getFrameBuffer(
//          {ETextureType::TEXTURE_CUBE, ETextureFormat::RGBA8, 2, 2, 1});
//      return temp->getTexture();
//    }
//    default:
//      JASSERT(0);
//      break;
//  }
//
//  return nullptr;
//}

FrameBufferPool::FrameBufferPool() {
}

FrameBufferPool::~FrameBufferPool() {
}

std::shared_ptr<FrameBuffer> FrameBufferPool::s_getFrameBuffer(
    const FrameBufferInfo& info) {
  auto hash = info.getHash();

  auto it_find = s_frameBufferResourceMap.find(hash);
  if (s_frameBufferResourceMap.end() != it_find) {
    auto& renderTargets = it_find->second;
    for (auto& iter : renderTargets) {
      if (!iter.m_isUsing_) {
        iter.m_isUsing_ = true;
        return iter.m_frameBufferPtr_;
      }
    }
  }

  auto renderTargetPtr
      = std::shared_ptr<FrameBuffer>(g_rhi->createFrameBuffer(info));
  if (renderTargetPtr) {
    s_frameBufferResourceMap[hash].push_back({true, renderTargetPtr});
    s_frameBufferHashVariableMap[renderTargetPtr.get()] = hash;
  }

  return renderTargetPtr;
}

void FrameBufferPool::s_returnFrameBuffer(FrameBuffer* renderTarget) {
  auto it_find = s_frameBufferHashVariableMap.find(renderTarget);
  if (s_frameBufferHashVariableMap.end() == it_find) {
    return;
  }

  const size_t hash = it_find->second;
  for (auto& iter : s_frameBufferResourceMap[hash]) {
    if (renderTarget == iter.m_frameBufferPtr_.get()) {
      iter.m_isUsing_ = false;
      break;
    }
  }
}

}  // namespace game_engine