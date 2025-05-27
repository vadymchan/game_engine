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

#ifdef GAME_ENGINE_TRACY_GPU_PROFILING_VK
#include "gfx/rhi/backends/vulkan/command_buffer_vk.h"

#include <tracy/TracyVulkan.hpp>

#define GPU_TRACY_ZONE_NC(cmdBuf, name, color)                                                        \
  TracyVkNamedZoneC(static_cast<TracyVkCtx>(ServiceLocator::s_get<gpu::GpuProfiler>()->getContext()), \
                    CONCAT(vk_zone_, __LINE__),                                                       \
                    static_cast<gfx::rhi::CommandBufferVk*>(cmdBuf)->getCommandBuffer(),              \
                    name,                                                                             \
                    ((color) >> 8),                                                                   \
                    true)

#define GPU_TRACY_ZONE_N(cmdBuf, name)                                                               \
  TracyVkNamedZone(static_cast<TracyVkCtx>(ServiceLocator::s_get<gpu::GpuProfiler>()->getContext()), \
                   CONCAT(vk_zone_, __LINE__),                                                       \
                   static_cast<gfx::rhi::CommandBufferVk*>(cmdBuf)->getCommandBuffer(),              \
                   name,                                                                             \
                   true)

#define GPU_TRACY_ZONE_C(cmdBuf, color)                                                          \
  TracyVkZoneC(static_cast<TracyVkCtx>(ServiceLocator::s_get<gpu::GpuProfiler>()->getContext()), \
               static_cast<gfx::rhi::CommandBufferVk*>(cmdBuf)->getCommandBuffer(),              \
               ((color) >> 8),                                                                   \
               true)

#elif defined(GAME_ENGINE_TRACY_GPU_PROFILING_DX12)
#include "gfx/rhi/backends/dx12/command_buffer_dx12.h"

#include <tracy/TracyD3D12.hpp>

#define GPU_TRACY_ZONE_NC(cmdBuf, name, color)                                                              \
  TracyD3D12NamedZoneC(static_cast<TracyD3D12Ctx>(ServiceLocator::s_get<gpu::GpuProfiler>()->getContext()), \
                       CONCAT(dx12_zone_, __LINE__),                                                        \
                       static_cast<gfx::rhi::CommandBufferDx12*>(cmdBuf)->getCommandList(),                 \
                       name,                                                                                \
                       ((color) >> 8),                                                                      \
                       true)

#define GPU_TRACY_ZONE_N(cmdBuf, name)                                                                     \
  TracyD3D12NamedZone(static_cast<TracyD3D12Ctx>(ServiceLocator::s_get<gpu::GpuProfiler>()->getContext()), \
                      CONCAT(dx12_zone_, __LINE__),                                                        \
                      static_cast<gfx::rhi::CommandBufferDx12*>(cmdBuf)->getCommandList(),                 \
                      name,                                                                                \
                      true)

#define GPU_TRACY_ZONE_C(cmdBuf, color)                                                                \
  TracyD3D12ZoneC(static_cast<TracyD3D12Ctx>(ServiceLocator::s_get<gpu::GpuProfiler>()->getContext()), \
                  static_cast<gfx::rhi::CommandBufferDx12*>(cmdBuf)->getCommandList(),                 \
                  ((color) >> 8),                                                                      \
                  true)

#else
#define GPU_TRACY_ZONE_NC(cmdBuf, name, color)
#define GPU_TRACY_ZONE_N(cmdBuf, name) 
#define GPU_TRACY_ZONE_C(cmdBuf, color) 
#endif

// main macros (client will call these)
#define GPU_ZONE_NC(cmdBuf, name, color)                                                   \
  auto CONCAT(_gpu_zone_, __LINE__) = ::game_engine::gpu::createZone(cmdBuf, name, color); \
  GPU_TRACY_ZONE_NC(cmdBuf, name, color)

#define GPU_ZONE_N(cmdBuf, name)                                                    \
  auto CONCAT(_gpu_zone_, __LINE__) = ::game_engine::gpu::createZone(cmdBuf, name); \
  GPU_TRACY_ZONE_N(cmdBuf, name)

#define GPU_ZONE_C(cmdBuf, color)                                                        \
  auto CONCAT(_gpu_zone_, __LINE__) = ::game_engine::gpu::createZone(cmdBuf, "", color); \
  GPU_TRACY_ZONE_C(cmdBuf, color)

#define GPU_MARKER(cmdBuf, name)          ::game_engine::gpu::insertMarker(cmdBuf, name)

#define GPU_MARKER_C(cmdBuf, name, color) ::game_engine::gpu::insertMarker(cmdBuf, name, color)

#endif  // GAME_ENGINE_NO_GPU_MACROS

#endif  // GAME_ENGINE_USE_GPU_PROFILING

#endif  // GAME_ENGINE_PROFILER_GPU_H