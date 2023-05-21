#include "i_window.h"


namespace GameEngine {

    class GlfwWindow : public IWindow<GlfwWindow> {
    public:
        GlfwWindow();
        ~GlfwWindow();

        bool InitializeImplementation(int width, int height, const std::string& title);
        void ShutdownImplementation();

        bool ShouldCloseImplementation() const;
        void PollEventsImplementation() const;
        void SwapBuffersImplementation() const;

        // other window-related methods...
    };

}  // namespace GameEngine

