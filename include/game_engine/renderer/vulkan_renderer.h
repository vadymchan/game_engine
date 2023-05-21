#pragma once


#include "i_renderer.h"
#include "../window/glfw_window.h"

namespace GameEngine {

    class VulkanRenderer : public IRenderer<VulkanRenderer> {
    public:
        VulkanRenderer();
        ~VulkanRenderer();

        bool InitializeImplementation(GlfwWindow& window);
        void ShutdownImplementation();

        void BeginFrameImplementation();
        void EndFrameImplementation();

        // other renderer-related methods...
    };

}  // namespace GameEngine


