// TODO: consider delete file
// #ifndef GAME_ENGINE_RENDER_TARGET_POOL_VK_H
// #define GAME_ENGINE_RENDER_TARGET_POOL_VK_H
//
// #include <list>
// #include <map>
// #include <memory>
//
// namespace game_engine {
//
// class RenderTargetVk;
// class RenderTargetInfoVk;
//
// class RenderTargetPoolVk {
//   public:
//   RenderTargetPoolVk() {}
//
//   ~RenderTargetPoolVk() {}
//
//   static std::shared_ptr<RenderTargetVk> GetRenderTarget(
//       const RenderTargetInfoVk& info);
//
//   static void ReturnRenderTarget(RenderTargetVk* renderTarget);
//
//   static void Release();
//
//   struct RenderTargetPoolResourceVk {
//     bool                            IsUsing = false;
//     std::shared_ptr<RenderTargetVk> RenderTargetPtr;
//   };
//
//   static std::map<size_t, std::list<RenderTargetPoolResourceVk> >
//                                            RenderTargetResourceMap;
//   static std::map<RenderTargetVk*, size_t> RenderTargetHashVariableMap;
// };
//
// }  // namespace game_engine
//
// #endif  // GAME_ENGINE_RENDER_TARGET_POOL_VK_H