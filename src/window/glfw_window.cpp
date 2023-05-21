#include "../include/game_engine/window/glfw_window.h"

#include <GLFW/glfw3.h>

namespace GameEngine {

    GLFWWindow::GLFWWindow() : window_(nullptr) {
        // Nothing to do here.
    }

    GLFWWindow::~GLFWWindow() {
        Shutdown();
    }

    bool GLFWWindow::Initialize(int width, int height, const std::string& title) {
        // Initialization code for GLFW window...
    }

    void GLFWWindow::Shutdown() {
        // Clean-up code for GLFW window...
    }

    bool GLFWWindow::ShouldClose() const {
        // Should close check for GLFW window...
    }

    void GLFWWindow::PollEvents() const {
        // Event polling for GLFW window...
    }

    void GLFWWindow::SwapBuffers() const {
        // Buffer swapping for GLFW window...
    }

}  // namespace GameEngine
