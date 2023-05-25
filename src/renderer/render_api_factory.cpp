// render_api_factory.cpp

#include "../include/game_engine/renderer/render_api_factory.h"
#include "../include/game_engine/renderer/dx12_renderer.h"
#include "../include/game_engine/renderer/vulkan_renderer.h"

namespace GameEngine {

    //std::unique_ptr<IRenderer> RenderAPIFactory::CreateRenderer(RenderAPI api) {
    //    switch (api) {
    //    case RenderAPI::DX12:
    //        return std::make_unique<DX12Renderer>();
    //    case RenderAPI::VULKAN:
    //        return std::make_unique<VulkanRenderer>();
    //        // add more cases as needed...
    //    default:
    //        // handle invalid API...
    //        return nullptr;
    //    }
    //}

}  // namespace GameEngine