#pragma once

#include "window.h"
#include <GLFW/glfw3.h>

namespace game_engine {
namespace window {

class GLFWWindow : public Window<GLFWWindow> {
public:
    using Window::Window;

    utils::ErrorCode InitializeImpl();
    bool ShouldCloseImpl() const;
    void PollEventsImpl();

private:
    GLFWwindow* window_ = nullptr;
};

} // namespace window
} // namespace game_engine
