#include "profiler/gpu.h"

#include "utils/logger/global_logger.h"

#ifdef GAME_ENGINE_USE_DX12
#include "gfx/rhi/backends/dx12/command_buffer_dx12.h"
#endif

#ifdef GAME_ENGINE_USE_VULKAN
#include "gfx/rhi/backends/vulkan/command_buffer_vk.h"
#endif

namespace game_engine {
namespace gpu {

//-------------------------------------------------------------------------
// ProfileZone implementation
//-------------------------------------------------------------------------

ProfileZone::~ProfileZone() {
  if (!m_cmdBuffer || !m_active) {
    GlobalLogger::Log(LogLevel::Error, "CommandBuffer is null or ProfileZone is not active, cannot end ProfileZone");
    return;
  }

  auto& profiler = detail::getProfiler();
  profiler.endZone(m_cmdBuffer);
}

namespace detail {

//-------------------------------------------------------------------------
// TracyProfiler implementation
//-------------------------------------------------------------------------

bool TracyProfiler::initializeContext(
    gfx::rhi::RenderingApi api, void* physicalDevice, void* device, void* queue, void* commandBuffer) {
  if (m_initialized) {
    GlobalLogger::Log(LogLevel::Warning, "Already initialized");
    return true;
  }

  m_api = api;

#ifdef GAME_ENGINE_USE_GPU_PROFILING
  switch (api) {
    case gfx::rhi::RenderingApi::Vulkan: {
      if (!physicalDevice || !device || !queue || !commandBuffer) {
        GlobalLogger::Log(LogLevel::Error, "Invalid Vulkan parameters");
        return false;
      }

      m_tracyVkContext = TracyVkContext(static_cast<VkPhysicalDevice>(physicalDevice),
                                        static_cast<VkDevice>(device),
                                        static_cast<VkQueue>(queue),
                                        static_cast<VkCommandBuffer>(commandBuffer));

      if (!m_tracyVkContext) {
        GlobalLogger::Log(LogLevel::Error, "Failed to create Tracy Vulkan context");
        return false;
      }

      GlobalLogger::Log(LogLevel::Info, "Tracy Vulkan context initialized");
      break;
    }

    case gfx::rhi::RenderingApi::Dx12: {
      if (!device || !queue) {
        GlobalLogger::Log(LogLevel::Error, "Invalid DirectX 12 parameters");
        return false;
      }

      m_tracyD3D12Context
          = TracyD3D12Context(static_cast<ID3D12Device*>(device), static_cast<ID3D12CommandQueue*>(queue));

      if (!m_tracyD3D12Context) {
        GlobalLogger::Log(LogLevel::Error, "Failed to create Tracy DirectX 12 context");
        return false;
      }

      GlobalLogger::Log(LogLevel::Info, "Tracy DirectX 12 context initialized");
      break;
    }

    default:
      GlobalLogger::Log(LogLevel::Error, "Unsupported API");
      return false;
  }
#else
  GlobalLogger::Log(LogLevel::Info, "Tracy profiling disabled, only native GPU markers will be used");
#endif

  m_initialized = true;
  return true;
}

void TracyProfiler::destroyContext() {
  if (!m_initialized) {
    return;
  }

#ifdef GAME_ENGINE_USE_GPU_PROFILING
  switch (m_api) {
    case gfx::rhi::RenderingApi::Vulkan:
      if (m_tracyVkContext) {
        TracyVkDestroy(m_tracyVkContext);
        m_tracyVkContext = nullptr;
        GlobalLogger::Log(LogLevel::Info, "Tracy Vulkan context destroyed");
      }
      break;

    case gfx::rhi::RenderingApi::Dx12:
      if (m_tracyD3D12Context) {
        TracyD3D12Destroy(m_tracyD3D12Context);
        m_tracyD3D12Context = nullptr;
        GlobalLogger::Log(LogLevel::Info, "Tracy DirectX 12 context destroyed");
      }
      break;
  }
#endif

  m_initialized = false;
}

void TracyProfiler::setContextName(const std::string& name) {
  if (!m_initialized) {
    return;
  }

#ifdef GAME_ENGINE_USE_GPU_PROFILING
  switch (m_api) {
    case gfx::rhi::RenderingApi::Vulkan:
      if (m_tracyVkContext) {
        TracyVkContextName(m_tracyVkContext, name.c_str(), name.size());
      }
      break;

    case gfx::rhi::RenderingApi::Dx12:
      if (m_tracyD3D12Context) {
        TracyD3D12ContextName(m_tracyD3D12Context, name.c_str(), name.size());
      }
      break;
  }
#endif
}

void TracyProfiler::newFrame() {
  if (!m_initialized) {
    return;
  }

#ifdef GAME_ENGINE_USE_GPU_PROFILING
  if (m_api == gfx::rhi::RenderingApi::Dx12 && m_tracyD3D12Context) {
    TracyD3D12NewFrame(m_tracyD3D12Context);
  }
#endif
}

void TracyProfiler::collect(void* commandBuffer) {
  if (!m_initialized) {
    return;
  }

#ifdef GAME_ENGINE_USE_GPU_PROFILING
  switch (m_api) {
    case gfx::rhi::RenderingApi::Vulkan:
      if (m_tracyVkContext && commandBuffer) {
        TracyVkCollect(m_tracyVkContext, static_cast<VkCommandBuffer>(commandBuffer));
      }
      break;

    case gfx::rhi::RenderingApi::Dx12:
      if (m_tracyD3D12Context) {
        TracyD3D12Collect(m_tracyD3D12Context);
      }
      break;
  }
#endif
}

void TracyProfiler::endZone(gfx::rhi::CommandBuffer* cmdBuffer) {
  if (!cmdBuffer) {
    return;
  }

  cmdBuffer->endDebugMarker();
}

void TracyProfiler::insertMarker(gfx::rhi::CommandBuffer* cmdBuffer, const std::string& name, const float* color) {
  if (!cmdBuffer) {
    return;
  }

  cmdBuffer->insertDebugMarker(name, color);
}

TracyProfiler& getProfiler() {
  auto* profiler = ServiceLocator::s_get<TracyProfiler>();
  if (!profiler) {
    static TracyProfiler fallbackProfiler;
    GlobalLogger::Log(LogLevel::Warning, "TracyProfiler not found in ServiceLocator, using fallback");
    return fallbackProfiler;
  }
  return *profiler;
}

}  // namespace detail

}  // namespace gpu
}  // namespace game_engine