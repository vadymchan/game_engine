#ifndef GAME_ENGINE_PROFILER_GPU_H
#define GAME_ENGINE_PROFILER_GPU_H

#include "profiler/backends/gpu_profiler_factory.h"
#include "utils/color/color.h"
#include "utils/logger/global_logger.h"
#include "utils/service/service_locator.h"

namespace game_engine {
namespace gpu {

class ProfileZone {
  public:
  template <uint32_t Color, size_t N>
  ProfileZone(gfx::rhi::CommandBuffer* cmdBuffer, const char (&name)[N])
      : m_cmdBuffer(cmdBuffer)
      , m_active(false) {
    if (!cmdBuffer) {
      GlobalLogger::Log(LogLevel::Error, "CommandBuffer is null, cannot create ProfileZone");
      return;
    }

    auto* profiler = ServiceLocator::s_get<GpuProfiler>();
    if (profiler) {
      profiler->beginZoneNC<Color>(cmdBuffer, name);
      m_active = true;
    }
  }

  template <uint32_t Color>
  ProfileZone(gfx::rhi::CommandBuffer* cmdBuffer)
      : m_cmdBuffer(cmdBuffer)
      , m_active(false) {
    if (!cmdBuffer) {
      GlobalLogger::Log(LogLevel::Error, "CommandBuffer is null, cannot create ProfileZone");
      return;
    }

    auto* profiler = ServiceLocator::s_get<GpuProfiler>();
    if (profiler) {
      profiler->beginZoneC<Color>(cmdBuffer);
      m_active = true;
    }
  }

  template <size_t N>
  ProfileZone(gfx::rhi::CommandBuffer* cmdBuffer, const char (&name)[N])
      : m_cmdBuffer(cmdBuffer)
      , m_active(false) {
    if (!cmdBuffer) {
      GlobalLogger::Log(LogLevel::Error, "CommandBuffer is null, cannot create ProfileZone");
      return;
    }

    auto* profiler = ServiceLocator::s_get<GpuProfiler>();
    if (profiler) {
      profiler->beginZoneN(cmdBuffer, name);
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

template <uint32_t Color, size_t N>
ProfileZone createZoneNC(gfx::rhi::CommandBuffer* cmdBuffer, const char (&name)[N]) {
  return ProfileZone<Color>(cmdBuffer, name);
}

template <uint32_t Color>
ProfileZone createZoneC(gfx::rhi::CommandBuffer* cmdBuffer) {
  return ProfileZone<Color>(cmdBuffer);
}

template <size_t N>
ProfileZone createZoneN(gfx::rhi::CommandBuffer* cmdBuffer, const char (&name)[N]) {
  return ProfileZone(cmdBuffer, name);
}

template <uint32_t Color, size_t N>
void insertMarker(gfx::rhi::CommandBuffer* cmdBuffer, const char (&name)[N]) {
  if (!cmdBuffer) {
    return;
  }

  auto* profiler = ServiceLocator::s_get<GpuProfiler>();
  if (profiler) {
    profiler->insertMarker<Color>(cmdBuffer, name);
  }
}

template <size_t N>
void insertMarker(gfx::rhi::CommandBuffer* cmdBuffer, const char (&name)[N]) {
  if (!cmdBuffer) {
    return;
  }

  auto* profiler = ServiceLocator::s_get<GpuProfiler>();
  if (profiler) {
    profiler->insertMarkerN(cmdBuffer, name);
  }
}

}  // namespace gpu
}  // namespace game_engine

#ifndef GAME_ENGINE_NO_GPU_MACROS

#define CONCAT_IMPL(x, y) x##y
#define CONCAT(x, y)      CONCAT_IMPL(x, y)

#define GPU_ZONE_NC(cmdBuffer, name, color) \
  auto CONCAT(_gpu_zone_, __LINE__) = ::game_engine::gpu::createZoneNC<color>(cmdBuffer, name)

#define GPU_ZONE_C(cmdBuffer, color) \
  auto CONCAT(_gpu_zone_, __LINE__) = ::game_engine::gpu::createZoneC<color>(cmdBuffer)

#define GPU_ZONE_N(cmdBuffer, name)          auto CONCAT(_gpu_zone_, __LINE__) = ::game_engine::gpu::createZoneN(cmdBuffer, name)

#define GPU_MARKER(cmdBuffer, name)          ::game_engine::gpu::insertMarker(cmdBuffer, name)

#define GPU_MARKER_C(cmdBuffer, name, color) ::game_engine::gpu::insertMarker<color>(cmdBuffer, name)

#endif  // GAME_ENGINE_NO_GPU_MACROS

#endif  // GAME_ENGINE_PROFILER_GPU_H