// render_api_factory.h

#pragma once

#include "i_renderer.h"
#include "dx12_renderer.h"
#include "vulkan_renderer.h"
#include "render_api_type.h"
#include "../window/i_window.h"
#include <memory>

namespace GameEngine {

    class RenderAPIFactory {
    public:
        static std::unique_ptr<IRenderer<DX12Renderer>> CreateDX12Renderer();
        static std::unique_ptr<IRenderer<VulkanRenderer>> CreateVulkanRenderer();
        //static std::unique_ptr<IRenderer> CreateRenderer(RenderAPI api); // currently not work due to CRTP pattern
    };

}  // namespace GameEngine
