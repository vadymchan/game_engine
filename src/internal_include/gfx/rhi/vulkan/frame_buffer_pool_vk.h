//#ifndef GAME_ENGINE_FRAME_BUFFER_POOL_VK_H
//#define GAME_ENGINE_FRAME_BUFFER_POOL_VK_H
//
// TODO: consider delete file
//#include "gfx/rhi/vulkan/frame_buffer_vk.h"
//
//#include <list>
//#include <map>
//#include <memory>
//
//namespace game_engine {
//class FrameBufferPoolVk {
//  public:
//  FrameBufferPoolVk();
//  ~FrameBufferPoolVk();
//
//  static std::shared_ptr<FrameBufferVk> GetFrameBuffer(
//      const FrameBufferInfoVk& info);
//
//  static void ReturnFrameBuffer(FrameBufferVk* renderTarget);
//
//  static void Release();
//
//  struct FrameBufferPoolResourceVk {
//    bool                           IsUsing = false;
//    std::shared_ptr<FrameBufferVk> FrameBufferPtr;
//  };
//
//  static std::map<size_t, std::list<FrameBufferPoolResourceVk> >
//                                          FrameBufferResourceMap;
//  static std::map<FrameBufferVk*, size_t> FrameBufferHashVariableMap;
//};
//
//}  // namespace game_engine
//
//#endif  // GAME_ENGINE_FRAME_BUFFER_POOL_VK_H