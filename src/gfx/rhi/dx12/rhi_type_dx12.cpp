
#include "gfx/rhi/dx12/rhi_type_dx12.h"
#include "gfx/rhi/dx12/rhi_dx12.h"


namespace game_engine {
void jCreatedResource::Free() {
  if (Resource) {
    if (ResourceType == jCreatedResource::EType::Standalone) {
      if (g_rhi_dx12) {
        g_rhi_dx12->DeallocatorMultiFrameStandaloneResource.Free(Resource);
      }
    } else if (ResourceType == jCreatedResource::EType::ResourcePool) {
      if (g_rhi_dx12) {
        g_rhi_dx12->DeallocatorMultiFramePlacedResource.Free(Resource);
      }
    } else if (ResourceType == jCreatedResource::EType::Swapchain) {
      // Nothing to do
    } else {
      assert(0);
    }
  }
}

}  // namespace game_engine