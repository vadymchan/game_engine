#ifndef GAME_ENGINE_PROFILER_GPU_H
#define GAME_ENGINE_PROFILER_GPU_H

#ifdef GAME_ENGINE_USE_GPU_PROFILING

#include "profiler/backends/gpu_profiler.h"
#include "profiler/backends/gpu_profiler_factory.h"
#include "utils/color/color.h"
#include "utils/logger/global_logger.h"
#include "utils/service/service_locator.h"

namespace game_engine {
namespace gpu {

class ProfileZone {
  public:
  ProfileZone(gfx::rhi::CommandBuffer* cmdBuffer, const std::string& name, uint32_t color = 0)
      : m_cmdBuffer(cmdBuffer)
      , m_active(false) {
    if (!cmdBuffer) {
      GlobalLogger::Log(LogLevel::Error, "CommandBuffer is null, cannot create ProfileZone");
      return;
    }

    auto* profiler = ServiceLocator::s_get<GpuProfiler>();
    if (profiler) {
      profiler->beginZone(cmdBuffer, name, color);
      m_active = true;
    }
  }

  ~ProfileZone() {
    if (!m_cmdBuffer || !m_active) {
      return;
    }

    auto* profiler = ServiceLocator::s_get<GpuProfiler>();
    if (profiler) {
      profiler->endZone(m_cmdBuffer);
    }
  }

  ProfileZone(const ProfileZone&)            = delete;
  ProfileZone& operator=(const ProfileZone&) = delete;
  ProfileZone(ProfileZone&&)                 = delete;
  ProfileZone& operator=(ProfileZone&&)      = delete;

  private:
  gfx::rhi::CommandBuffer* m_cmdBuffer;
  bool                     m_active;
};

inline ProfileZone createZone(gfx::rhi::CommandBuffer* cmdBuffer, const std::string& name, uint32_t color = 0) {
  return ProfileZone(cmdBuffer, name, color);
}

inline void insertMarker(gfx::rhi::CommandBuffer* cmdBuffer, const std::string& name, uint32_t color = 0) {
  if (!cmdBuffer) {
    return;
  }

  auto* profiler = ServiceLocator::s_get<GpuProfiler>();
  if (profiler) {
    profiler->insertMarker(cmdBuffer, name, color);
  }
}

}  // namespace gpu
}  // namespace game_engine

#ifndef GAME_ENGINE_NO_GPU_MACROS

#define CONCAT_IMPL(x, y) x##y
#define CONCAT(x, y)      CONCAT_IMPL(x, y)

// helper
#define GPU_ZONE_DISPATCH(Func, cmdBuf, name, color)                                                \
  do {                                                                                              \
    auto* gpuProfiler = ServiceLocator::s_get<gpu::GpuProfiler>();                                  \
    if (!gpuProfiler) {                                                                             \
      GlobalLogger::Log(LogLevel::Warning, "GPU profiler not found");                               \
      break;                                                                                        \
    }                                                                                               \
    using Api = gfx::rhi::RenderingApi;                                                             \
    switch (gpuProfiler->getApi()) {                                                                \
      case Api::Vulkan: {                                                                           \
        Func##_VK(gpuProfiler, cmdBuf, name, color);                                                \
      } break;                                                                                      \
      case Api::Dx12: {                                                                             \
        Func##_DX12(gpuProfiler, cmdBuf, name, color);                                              \
      } break;                                                                                      \
      default: {                                                                                    \
        GlobalLogger::Log(LogLevel::Error, "GPU profiler implementation for this API was removed"); \
      }                                                                                             \
    }                                                                                               \
  } while (false)

// vulkan
#ifdef PROFILER_GPU_VK_ENABLED
#include "gfx/rhi/backends/vulkan/command_buffer_vk.h"

#include <tracy/TracyVulkan.hpp>

#define GPU_ZONE_DETAIL_FUNC_NC_VK(profiler, cmdBuf, name, color)                        \
  TracyVkNamedZoneC(static_cast<TracyVkCtx>(profiler->getContext()),                     \
                    CONCAT(vk##_zone_, __LINE__),                                       \
                    static_cast<gfx::rhi::CommandBufferVk*>(cmdBuf)->getCommandBuffer(), \
                    name,                                                                \
                    ((color) >> 8),                                                      \
                    true);

#define GPU_ZONE_DETAIL_FUNC_N_VK(profiler, cmdBuf, name, unused1)                      \
  TracyVkNamedZone(static_cast<TracyVkCtx>(profiler->getContext()),                     \
                   CONCAT(vk##_zone_, __LINE__),                                        \
                   static_cast<gfx::rhi::CommandBufferVk*>(cmdBuf)->getCommandBuffer(), \
                   name,                                                                \
                   true);

#define GPU_ZONE_DETAIL_FUNC_C_VK(profiler, cmdBuf, unused1, color, unused2)        \
  TracyVkZoneC(static_cast<TracyVkCtx>(profiler->getContext()),                     \
               static_cast<gfx::rhi::CommandBufferVk*>(cmdBuf)->getCommandBuffer(), \
               ((color) >> 8),                                                      \
               true);
#else
#define GPU_ZONE_DETAIL_FUNC_NC_VK(...) nullptr
#define GPU_ZONE_DETAIL_FUNC_N_VK(...)  nullptr
#define GPU_ZONE_DETAIL_FUNC_C_VK(...)  nullptr
#endif

// dx12
#ifdef PROFILER_GPU_DX12_ENABLED
#include "gfx/rhi/backends/dx12/command_buffer_dx12.h"

#include <tracy/TracyD3D12.hpp>

#define GPU_ZONE_DETAIL_FUNC_NC_DX12(profiler, cmdBuf, name, color)                         \
  TracyD3D12NamedZoneC(static_cast<TracyD3D12Ctx>(profiler->getContext()),                  \
                       CONCAT(dx12##_zone_, __LINE__),                                      \
                       static_cast<gfx::rhi::CommandBufferDx12*>(cmdBuf)->getCommandList(), \
                       name,                                                                \
                       ((color) >> 8),                                                      \
                       true);

#define GPU_ZONE_DETAIL_FUNC_N_DX12(profiler, cmdBuf, name, unused1)                       \
  TracyD3D12NamedZone(static_cast<TracyD3D12Ctx>(profiler->getContext()),                  \
                      CONCAT(dx12##_zone_, __LINE__),                                      \
                      static_cast<gfx::rhi::CommandBufferDx12*>(cmdBuf)->getCommandList(), \
                      name,                                                                \
                      true);

#define GPU_ZONE_DETAIL_FUNC_C_DX12(profiler, cmdBuf, unused1, color, unused2)         \
  TracyD3D12ZoneC(static_cast<TracyD3D12Ctx>(profiler->getContext()),                  \
                  static_cast<gfx::rhi::CommandBufferDx12*>(cmdBuf)->getCommandList(), \
                  ((color) >> 8),                           \
                  true);
#else
#define GPU_ZONE_DETAIL_FUNC_NC_DX12(...) nullptr
#define GPU_ZONE_DETAIL_FUNC_N_DX12(...)  nullptr
#define GPU_ZONE_DETAIL_FUNC_C_DX12(...)  nullptr
#endif

// main macros (client will call these)
#define GPU_ZONE_NC(cmdBuf, name, color)                                                   \
  auto CONCAT(_gpu_zone_, __LINE__) = ::game_engine::gpu::createZone(cmdBuf, name, color); \
  GPU_ZONE_DISPATCH(GPU_ZONE_DETAIL_FUNC_NC, cmdBuf, name, color)

#define GPU_ZONE_N(cmdBuf, name)                                                    \
  auto CONCAT(_gpu_zone_, __LINE__) = ::game_engine::gpu::createZone(cmdBuf, name); \
  GPU_ZONE_DISPATCH(GPU_ZONE_DETAIL_FUNC_N, cmdBuf, name, 0)

#define GPU_ZONE_C(cmdBuf, color)                                                        \
  auto CONCAT(_gpu_zone_, __LINE__) = ::game_engine::gpu::createZone(cmdBuf, "", color); \
  GPU_ZONE_DISPATCH(GPU_ZONE_DETAIL_FUNC_C, cmdBuf, std::string{}, color)

#define GPU_MARKER(cmdBuf, name)          ::game_engine::gpu::insertMarker(cmdBuf, name);

#define GPU_MARKER_C(cmdBuf, name, color) ::game_engine::gpu::insertMarker(cmdBuf, name, color);

#endif  // GAME_ENGINE_NO_GPU_MACROS

#endif  // GAME_ENGINE_USE_GPU_PROFILING

#endif  // GAME_ENGINE_PROFILER_GPU_H