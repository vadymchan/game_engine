#pragma once

#include <string>

namespace GameEngine {

    class IWindow {
    public:
        virtual ~IWindow() = default;

        virtual bool Initialize(int width, int height, const std::string& title) = 0;
        virtual void Shutdown() = 0;

        virtual bool ShouldClose() const = 0;
        virtual void PollEvents() const = 0;
        virtual void SwapBuffers() const = 0;
    };

}  // namespace GameEngine

