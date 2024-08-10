//
//#include "gfx/rhi/vulkan/frame_buffer_pool_vk.h"
//
//#include "gfx/rhi/vulkan/rhi_vk.h"
//
//namespace game_engine {
//
//std::map<uint64_t, std::list<FrameBufferPoolVk::FrameBufferPoolResourceVk>>
//    FrameBufferPoolVk::FrameBufferResourceMap;
//std::map<FrameBufferVk*, uint64_t>
//    FrameBufferPoolVk::FrameBufferHashVariableMap;
//
//std::shared_ptr<FrameBufferVk> FrameBufferPoolVk::GetFrameBuffer(
//    const FrameBufferInfoVk& info) {
//  auto hash = info.GetHash();
//
//  auto it_find = FrameBufferResourceMap.find(hash);
//  if (FrameBufferResourceMap.end() != it_find) {
//    auto& renderTargets = it_find->second;
//    for (auto& iter : renderTargets) {
//      if (!iter.IsUsing) {
//        iter.IsUsing = true;
//        return iter.FrameBufferPtr;
//      }
//    }
//  }
//
//  auto renderTargetPtr
//      = std::shared_ptr<FrameBufferVk>(g_rhi_vk->CreateFrameBuffer(info));
//  if (renderTargetPtr) {
//    FrameBufferResourceMap[hash].push_back({true, renderTargetPtr});
//    FrameBufferHashVariableMap[renderTargetPtr.get()] = hash;
//  }
//
//  return renderTargetPtr;
//}
//
//void FrameBufferPoolVk::ReturnFrameBuffer(FrameBufferVk* renderTarget) {
//  auto it_find = FrameBufferHashVariableMap.find(renderTarget);
//  if (FrameBufferHashVariableMap.end() == it_find) {
//    return;
//  }
//
//  const size_t hash = it_find->second;
//  for (auto& iter : FrameBufferResourceMap[hash]) {
//    if (renderTarget == iter.FrameBufferPtr.get()) {
//      iter.IsUsing = false;
//      break;
//    }
//  }
//}
//
//void FrameBufferPoolVk::Release() {
//  FrameBufferResourceMap.clear();
//  FrameBufferHashVariableMap.clear();
//}
//
//}  // namespace game_engine
