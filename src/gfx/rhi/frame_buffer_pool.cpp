#include "gfx/rhi/frame_buffer_pool.h"

#include "gfx/rhi/rhi.h"

namespace game_engine {

std::map<size_t, std::list<FrameBufferPool::FrameBufferPoolResource> >
                                FrameBufferPool::FrameBufferResourceMap;
std::map<FrameBuffer*, size_t> FrameBufferPool::FrameBufferHashVariableMap;

//struct Texture* FrameBufferPool::GetNullTexture(ETextureType type) {
//  switch (type) {
//    case ETextureType::TEXTURE_2D: {
//      static auto temp = FrameBufferPool::GetFrameBuffer(
//          {ETextureType::TEXTURE_2D, ETextureFormat::RGBA8, 2, 2, 1});
//      return temp->GetTexture();
//    }
//    case ETextureType::TEXTURE_2D_ARRAY: {
//      static auto temp = FrameBufferPool::GetFrameBuffer(
//          {ETextureType::TEXTURE_2D_ARRAY, ETextureFormat::RGBA8, 2, 2, 1});
//      return temp->GetTexture();
//    }
//    case ETextureType::TEXTURE_CUBE: {
//      static auto temp = FrameBufferPool::GetFrameBuffer(
//          {ETextureType::TEXTURE_CUBE, ETextureFormat::RGBA8, 2, 2, 1});
//      return temp->GetTexture();
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

std::shared_ptr<FrameBuffer> FrameBufferPool::GetFrameBuffer(
    const FrameBufferInfo& info) {
  auto hash = info.GetHash();

  auto it_find = FrameBufferResourceMap.find(hash);
  if (FrameBufferResourceMap.end() != it_find) {
    auto& renderTargets = it_find->second;
    for (auto& iter : renderTargets) {
      if (!iter.IsUsing) {
        iter.IsUsing = true;
        return iter.FrameBufferPtr;
      }
    }
  }

  auto renderTargetPtr
      = std::shared_ptr<FrameBuffer>(g_rhi->CreateFrameBuffer(info));
  if (renderTargetPtr) {
    FrameBufferResourceMap[hash].push_back({true, renderTargetPtr});
    FrameBufferHashVariableMap[renderTargetPtr.get()] = hash;
  }

  return renderTargetPtr;
}

void FrameBufferPool::ReturnFrameBuffer(FrameBuffer* renderTarget) {
  auto it_find = FrameBufferHashVariableMap.find(renderTarget);
  if (FrameBufferHashVariableMap.end() == it_find) {
    return;
  }

  const size_t hash = it_find->second;
  for (auto& iter : FrameBufferResourceMap[hash]) {
    if (renderTarget == iter.FrameBufferPtr.get()) {
      iter.IsUsing = false;
      break;
    }
  }
}

}  // namespace game_engine