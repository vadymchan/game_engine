// #pragma once

// #include "render_api.h"
// #include "dx11_render_api.h"
// #include "dx12_render_api.h"
// #include "vulkan_render_api.h"

// namespace game_engine {
// namespace renderer {

// std::unique_ptr<RenderAPI> CreateRenderAPI(RenderAPIType type) {
//     switch (type) {
//         case
// RenderAPIType::DirectX11:
// return std::make_unique<DX11RenderAPI>();
// case RenderAPIType::DirectX12:
// return std::make_unique<DX12RenderAPI>();
// case RenderAPIType::Vulkan:
// return std::make_unique<VulkanRenderAPI>();
// default:
// return nullptr;
// }
// }

// } // namespace renderer
// } // namespace game_engine

#pragma once

#include "render_api.h"
#include "dx11_render_api.h"
#include "dx12_render_api.h"
#include "vulkan_render_api.h"

namespace game_engine {
namespace renderer {

template <typename T>
std::unique_ptr<RenderAPI<T>> CreateRenderAPI() {
    return std::make_unique<T>();
}

} // namespace renderer
} // namespace game_engine
