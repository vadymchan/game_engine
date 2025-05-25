#include "profiler/backends/gpu_profiler_vk.h"

#ifdef GAME_ENGINE_USE_VULKAN

#include "gfx/rhi/backends/vulkan/command_buffer_vk.h"
#include "utils/logger/global_logger.h"

namespace game_engine {
namespace gpu {

bool GpuProfilerVk::initialize(void* physicalDevice, void* device, void* queue, void* commandBuffer) {
  if (m_initialized) {
    GlobalLogger::Log(LogLevel::Warning, "GpuProfilerVk already initialized");
    return true;
  }

  if (!physicalDevice || !device || !queue || !commandBuffer) {
    GlobalLogger::Log(LogLevel::Error, "Invalid Vulkan parameters for profiler");
    return false;
  }

#ifdef GAME_ENGINE_USE_GPU_PROFILING
  m_tracyContext = TracyVkContext(static_cast<VkPhysicalDevice>(physicalDevice),
                                  static_cast<VkDevice>(device),
                                  static_cast<VkQueue>(queue),
                                  static_cast<VkCommandBuffer>(commandBuffer));

  if (!m_tracyContext) {
    GlobalLogger::Log(LogLevel::Error, "Failed to create Tracy Vulkan context");
    return false;
  }

  GlobalLogger::Log(LogLevel::Info, "Tracy Vulkan profiler initialized");
#else
  GlobalLogger::Log(LogLevel::Info, "Tracy profiling disabled, using native GPU markers only");
#endif

  setApi(gfx::rhi::RenderingApi::Vulkan);
  m_initialized = true;
  return true;
}

void GpuProfilerVk::destroy() {
  if (!m_initialized) {
    return;
  }

#ifdef GAME_ENGINE_USE_GPU_PROFILING
  if (m_tracyContext) {
    TracyVkDestroy(m_tracyContext);
    m_tracyContext = nullptr;
    GlobalLogger::Log(LogLevel::Info, "Tracy Vulkan profiler destroyed");
  }
#endif

  m_initialized = false;
}

void GpuProfilerVk::setContextName(const std::string& name) {
  if (!m_initialized) {
    return;
  }

#ifdef GAME_ENGINE_USE_GPU_PROFILING
  if (m_tracyContext) {
    TracyVkContextName(m_tracyContext, name.c_str(), name.size());
  }
#endif
}

void GpuProfilerVk::collect(void* commandBuffer) {
  if (!m_initialized) {
    return;
  }

#ifdef GAME_ENGINE_USE_GPU_PROFILING
  if (m_tracyContext && commandBuffer) {
    TracyVkCollect(m_tracyContext, static_cast<VkCommandBuffer>(commandBuffer));
  }
#endif
}

void GpuProfilerVk::endZone(gfx::rhi::CommandBuffer* cmdBuffer) {
  if (!cmdBuffer) {
    return;
  }
  cmdBuffer->endDebugMarker();
}

}  // namespace gpu
}  // namespace game_engine

#endif  // GAME_ENGINE_USE_VULKAN