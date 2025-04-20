#ifndef GAME_ENGINE_RHI_FACTORY_H
#define GAME_ENGINE_RHI_FACTORY_H

#include "gfx/rhi/common/rhi_enums.h"
#include "gfx/rhi/common/rhi_types.h"

#include <memory>

namespace game_engine {
namespace gfx {
namespace rhi {

class Device;

std::unique_ptr<Device> g_createDevice(RenderingApi api, const DeviceDesc& desc);

}  // namespace rhi
}  // namespace gfx
}  // namespace game_engine

#endif  // GAME_ENGINE_RHI_FACTORY_H