#pragma once

#include "i_renderer.h"

namespace GameEngine {

    class VulkanRenderer : public IRenderer {
    public:
        VulkanRenderer();
        virtual ~VulkanRenderer();

        bool Initialize(IWindow& window) override;
        void Shutdown() override;

        void BeginFrame() override;
        void EndFrame() override;

        // other renderer-related methods...
    };
}  // namespace GameEngine
