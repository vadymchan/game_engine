#ifndef GAME_ENGINE_GPU_PROFILER_INL
#define GAME_ENGINE_GPU_PROFILER_INL
#include "profiler/backends/gpu_profiler.h"

namespace game_engine {
namespace gpu {

template <uint32_t Color, size_t N>
inline void GpuProfiler::beginZoneNC(gfx::rhi::CommandBuffer* cmdBuf, const char (&name)[N]) {
  if (!cmdBuf) {
    return;
  }
  auto colorArr = color::g_toFloatArray(Color);
  cmdBuf->beginDebugMarker(name, colorArr.data());

#if defined(GAME_ENGINE_USE_GPU_PROFILING) && defined(TRACY_ENABLE)
  switch (m_api) {
#if defined(GAME_ENGINE_USE_VULKAN)
#if defined(GAME_ENGINE_GPU_PROFILER_VK_H)
    case gfx::rhi::RenderingApi::Vulkan:
      static_cast<GpuProfilerVk*>(this)->template beginZoneNC<Color>(cmdBuf, name);
      break;
#endif
#endif
#if defined(GAME_ENGINE_USE_DX12)
#if defined(GAME_ENGINE_GPU_PROFILER_DX12_H)
    case gfx::rhi::RenderingApi::Dx12:
      static_cast<GpuProfilerDx12*>(this)->template beginZoneNC<Color>(cmdBuf, name);

      break;
#endif
#endif
    default:
      break;
  }
#endif
}

template <uint32_t Color>
inline void GpuProfiler::beginZoneC(gfx::rhi::CommandBuffer* cmdBuf) {
  if (!cmdBuf) {
    return;
  }
  auto colorArr = color::g_toFloatArray(Color);
  cmdBuf->beginDebugMarker("", colorArr.data());

#if defined(GAME_ENGINE_USE_GPU_PROFILING) && defined(TRACY_ENABLE)
  switch (m_api) {
#if defined(GAME_ENGINE_USE_VULKAN)
#if defined(GAME_ENGINE_GPU_PROFILER_VK_H)
    case gfx::rhi::RenderingApi::Vulkan:
      static_cast<GpuProfilerVk*>(this)->template beginZoneC<Color>(cmdBuf);
      break;
#endif
#endif
#if defined(GAME_ENGINE_USE_DX12)
#if defined(GAME_ENGINE_GPU_PROFILER_DX12_H)
    case gfx::rhi::RenderingApi::Dx12:
      static_cast<GpuProfilerDx12*>(this)->template beginZoneC<Color>(cmdBuf);
      break;
#endif
#endif
    default:
      break;
  }
#endif
}

template <size_t N>
inline void GpuProfiler::beginZoneN(gfx::rhi::CommandBuffer* cmdBuf, const char (&name)[N]) {
  if (!cmdBuf) {
    return;
  }
  cmdBuf->beginDebugMarker(name, nullptr);

#if defined(GAME_ENGINE_USE_GPU_PROFILING) && defined(TRACY_ENABLE)
  switch (m_api) {
#if defined(GAME_ENGINE_USE_VULKAN)
#if defined(GAME_ENGINE_GPU_PROFILER_VK_H)
    case gfx::rhi::RenderingApi::Vulkan:
      static_cast<GpuProfilerVk*>(this)->beginZoneN(cmdBuf, name);
      break;
#endif
#endif
#if defined(GAME_ENGINE_USE_DX12)
#if defined(GAME_ENGINE_GPU_PROFILER_DX12_H)
    case gfx::rhi::RenderingApi::Dx12:
      static_cast<GpuProfilerDx12*>(this)->beginZoneN(cmdBuf, name);
      break;
#endif
#endif
    default:
      break;
  }
#endif
}

}  // namespace gpu
}  // namespace game_engine

#endif  // GAME_ENGINE_GPU_PROFILER_INL