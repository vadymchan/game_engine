#include "glfw_window.h" // change the path

namespace game_engine {
namespace window {

utils::ErrorCode GLFWWindow::InitializeImpl() {
    if (!glfwInit()) {
        return utils::ErrorCode::GLFWInitFailed;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window_ = glfwCreateWindow(width_, height_, title_.c_str(), nullptr, nullptr);

    if (!window_) {
        glfwTerminate();
        return utils::ErrorCode::GLFWWindowCreationFailed;
    }

    return utils::ErrorCode::Success;
}

bool GLFWWindow::ShouldCloseImpl() const {
    return glfwWindowShouldClose(window_);
}

void GLFWWindow::PollEventsImpl() {
    glfwPollEvents();
}

} // namespace window
} // namespace game_engine
