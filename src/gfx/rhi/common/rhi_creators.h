#ifndef ARISE_RHI_FACTORY_H
#define ARISE_RHI_FACTORY_H

#include "gfx/rhi/common/rhi_enums.h"
#include "gfx/rhi/common/rhi_types.h"

#include <memory>

namespace arise {
namespace gfx {
namespace rhi {

class Device;

std::unique_ptr<Device> g_createDevice(RenderingApi api, const DeviceDesc& desc);

}  // namespace rhi
}  // namespace gfx
}  // namespace arise

#endif  // ARISE_RHI_FACTORY_H