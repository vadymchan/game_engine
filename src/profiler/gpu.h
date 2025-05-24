#ifndef GAME_ENGINE_PROFILER_GPU_H
#define GAME_ENGINE_PROFILER_GPU_H

#include "gfx/rhi/common/rhi_enums.h"
#include "gfx/rhi/interface/command_buffer.h"
#include "utils/color/color.h"
#include "utils/logger/global_logger.h"
#include "utils/service/service_locator.h"


// TODO: organize includes better
#ifdef GAME_ENGINE_USE_GPU_PROFILING
#include <tracy/Tracy.hpp>

#ifdef GAME_ENGINE_USE_DX12
#include <tracy/TracyD3D12.hpp>
using TracyD3D12ContextType = TracyD3D12Ctx;
#endif

#ifdef GAME_ENGINE_USE_VULKAN
#include <vulkan/vulkan.h>

#include <tracy/TracyVulkan.hpp>
using TracyVkContextType = TracyVkCtx;
#endif

#else
using TracyVkContextType    = void*;
using TracyD3D12ContextType = void*;
#endif

namespace game_engine {
namespace gpu {

class ProfileZone {
  public:
  template <uint32_t Color = color::WHITE, size_t N>
  ProfileZone(gfx::rhi::CommandBuffer* cmdBuffer, const char (&name)[N])
      : m_cmdBuffer(cmdBuffer)
      , m_active(false) {
    if (!cmdBuffer) {
      GlobalLogger::Log(LogLevel::Error, "CommandBuffer is null, cannot create ProfileZone");
      return;
    }

    auto& profiler = detail::getProfiler();
    profiler.beginZone<Color>(cmdBuffer, name);
    m_active = true;
  }

  ~ProfileZone();

  ProfileZone(const ProfileZone&)            = delete;
  ProfileZone& operator=(const ProfileZone&) = delete;
  ProfileZone(ProfileZone&&)                 = delete;
  ProfileZone& operator=(ProfileZone&&)      = delete;

  private:
  gfx::rhi::CommandBuffer* m_cmdBuffer;
  bool                     m_active;
};

// TODO: add createZone that is not colored

template <uint32_t Color, size_t N>
ProfileZone createZone(gfx::rhi::CommandBuffer* cmdBuffer, const char (&name)[N]) {
  return ProfileZone<Color>(cmdBuffer, name);
}

// TODO: add insertMarker that is not colored

template <uint32_t Color, size_t N>
void insertMarker(gfx::rhi::CommandBuffer* cmdBuffer, const char (&name)[N]) {
  if (!cmdBuffer) {
    return;
  }

  auto& profiler = detail::getProfiler();
  profiler.insertMarker<Color>(cmdBuffer, name);
}

namespace detail {

class TracyProfiler {
  public:
  TracyProfiler()  = default;
  ~TracyProfiler() = default;

  TracyProfiler(const TracyProfiler&)            = delete;
  TracyProfiler& operator=(const TracyProfiler&) = delete;

  TracyProfiler(TracyProfiler&&) noexcept            = default;
  TracyProfiler& operator=(TracyProfiler&&) noexcept = default;

  bool initializeContext(
      gfx::rhi::RenderingApi api, void* physicalDevice, void* device, void* queue, void* commandBuffer = nullptr);
  void destroyContext();
  void setContextName(const std::string& name);

  void newFrame();
  void collect(void* commandBuffer = nullptr);

  template <uint32_t Color = color::WHITE, size_t N>
  void beginZone(gfx::rhi::CommandBuffer* cmdBuffer, const char (&name)[N]);
  void endZone(gfx::rhi::CommandBuffer* cmdBuffer);

  template <uint32_t Color = color::WHITE, size_t N>
  void insertMarker(gfx::rhi::CommandBuffer* cmdBuffer, const char (&name)[N]);
  void insertMarker(gfx::rhi::CommandBuffer* cmdBuffer, const std::string& name, const float* color = nullptr);

  bool                   isInitialized() const { return m_initialized; }
  gfx::rhi::RenderingApi getApi() const { return m_api; }

  private:
  gfx::rhi::RenderingApi m_api         = gfx::rhi::RenderingApi::Vulkan;
  bool                   m_initialized = false;

  TracyVkContextType    m_tracyVkContext    = nullptr;
  TracyD3D12ContextType m_tracyD3D12Context = nullptr;
};

template <uint32_t Color, size_t N>
void TracyProfiler::beginZone(gfx::rhi::CommandBuffer* cmdBuffer, const char (&name)[N]) {
  if (!cmdBuffer) {
    return;
  }

  auto colorArray = color::g_toFloatArray(Color);
  cmdBuffer->beginDebugMarker(name, colorArray.data());

#if defined(GAME_ENGINE_USE_GPU_PROFILING) && defined(TRACY_ENABLE)
  switch (m_api) {
    case gfx::rhi::RenderingApi::Vulkan: {
#ifdef GAME_ENGINE_USE_VULKAN
      auto* cmdBufferVk = static_cast<gfx::rhi::CommandBufferVk*>(cmdBuffer);
      TracyVkNamedZoneC(m_tracyVkContext, _zone, cmdBufferVk->getCommandBuffer(), name, Color, true);
#endif
      break;
    }
    case gfx::rhi::RenderingApi::Dx12: {
#ifdef GAME_ENGINE_USE_DX12
      auto* cmdBufferDx12 = static_cast<gfx::rhi::CommandBufferDx12*>(cmdBuffer);
      TracyD3D12NamedZoneC(m_tracyD3D12Context, _zone, cmdBufferDx12->getCommandList(), name, Color, true);
#endif
      break;
    }
  }
#endif
}

template <uint32_t Color, size_t N>
void TracyProfiler::insertMarker(gfx::rhi::CommandBuffer* cmdBuffer, const char (&name)[N]) {
  if (!cmdBuffer) {
    return;
  }

  auto colorArray = color::g_toFloatArray(Color);
  cmdBuffer->insertDebugMarker(name, colorArray.data());
}

TracyProfiler& getProfiler();

}  // namespace detail

}  // namespace gpu
}  // namespace game_engine

#ifndef GAME_ENGINE_NO_GPU_MACROS

#define CONCAT_IMPL(x, y)         x##y
#define CONCAT(x, y)              CONCAT_IMPL(x, y)

// Simplified universal macros
#define GPU_ZONE(cmdBuffer, name) auto CONCAT(_gpu_zone_, __LINE__) = ::game_engine::gpu::createZone(cmdBuffer, name)

#define GPU_ZONE_COLORED(cmdBuffer, name, color) \
  auto CONCAT(_gpu_zone_, __LINE__) = ::game_engine::gpu::createZone<color>(cmdBuffer, name)

#define GPU_MARKER(cmdBuffer, name)                ::game_engine::gpu::insertMarker(cmdBuffer, name)

#define GPU_MARKER_COLORED(cmdBuffer, name, color) ::game_engine::gpu::insertMarker<color>(cmdBuffer, name)

#endif  // GAME_ENGINE_NO_GPU_MACROS

#endif  // GAME_ENGINE_PROFILER_GPU_H