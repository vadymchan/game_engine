#include "gfx/rhi/common/rhi_creators.h"
#include "gfx/rhi/interface/device.h"

//#ifdef GAME_ENGINE_RHI_VULKAN
#include "gfx/rhi/backends/vulkan/device_vk.h"
//#endif

#include "platform/windows/windows_platform_setup.h"

#ifdef GAME_ENGINE_RHI_DX12
#include "gfx/rhi/backends/dx12/device_dx12.h"
#endif

#include <cassert>

namespace game_engine {
namespace gfx {
namespace rhi {

std::unique_ptr<Device> g_createDevice(RenderingApi api, const DeviceDesc& desc) {
  std::unique_ptr<Device> device;

  switch (api) {
    case RenderingApi::Vulkan:
      device = std::make_unique<DeviceVk>(desc);
      break;
#ifdef GAME_ENGINE_RHI_DX12
    case RenderingApi::Dx12:
      device = std::make_unique<DeviceDx12>(desc);
      break;
#endif
    default:
      assert(false && "Unsupported rendering API");
      return nullptr;
  }

  if (!device) {
    assert(false && "Failed to create device");
    return nullptr;
  }

  return device;
}

}  // namespace rhi
}  // namespace gfx
}  // namespace game_engine