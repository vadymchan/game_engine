
#include "../include/game_engine/window/window_factory.h"




namespace GameEngine
{
    class WindowFactory {
    public:
        static std::unique_ptr<IBaseWindow> CreateGameWindow(RenderAPI api) {
            switch (api) {
            case RenderAPI::DX12:
                return CreateWinApiWindow();
                break;
            case RenderAPI::VULKAN:
                return CreateGLFWindow();
                break;
            default:
				return nullptr;
				break;

            }
        }

        static std::unique_ptr<IWindow<GlfwWindow>> CreateGLFWindow() {
            return std::make_unique<GlfwWindow>();
        }

        static std::unique_ptr<IWindow<WinApiWindow>> CreateWinApiWindow() {
            return std::make_unique<WinApiWindow>();
        }
    };

}

