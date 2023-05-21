#include "i_window.h"

struct GLFWwindow;

namespace GameEngine {

    class GLFWWindow : public IWindow {
    public:
        GLFWWindow();
        virtual ~GLFWWindow();

        bool Initialize(int width, int height, const std::string& title) override;
        void Shutdown() override;

        bool ShouldClose() const override;
        void PollEvents() const override;
        void SwapBuffers() const override;

    private:
        GLFWwindow* window_;
    };

}  // namespace GameEngine

