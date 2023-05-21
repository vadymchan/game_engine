#pragma once


#include <string>

namespace GameEngine {

    template <typename DerivedWindow>
    class IWindow {
    public:
        bool Initialize(int width, int height, const std::string& title) {
            return static_cast<DerivedWindow*>(this)->InitializeImplementation(width, height, title);
        }

        void Shutdown() {
            return static_cast<DerivedWindow*>(this)->ShutdownImplementation();
        }

        bool ShouldClose() const {
            return static_cast<const DerivedWindow*>(this)->ShouldCloseImplementation();
        }

        void PollEvents() const {
            return static_cast<const DerivedWindow*>(this)->PollEventsImplementation();
        }

        void SwapBuffers() const {
            return static_cast<const DerivedWindow*>(this)->SwapBuffersImplementation();
        }
    };

}  // namespace GameEngine

