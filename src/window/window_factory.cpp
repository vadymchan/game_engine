//
//#include "../include/game_engine/window/window_factory.h"
//
//
//
//
//namespace GameEngine
//{
//
//    std::unique_ptr<IWindow<IBaseWindow>> WindowFactory::CreateGLFWindow() {
//        return std::make_unique<GlfwWindow>();
//    }
//
//    std::unique_ptr<IWindow<IBaseWindow>> WindowFactory::CreateWinApiWindow() {
//        return std::make_unique<WinApiWindow>();
//    }
//
//    std::unique_ptr<IWindow<IBaseWindow>> WindowFactory::CreateGameWindow(RenderAPI api) {
//        switch (api) {
//        case RenderAPI::DX12:
//            return CreateWinApiWindow();
//            break;
//        case RenderAPI::VULKAN:
//            return CreateGLFWindow();
//            break;
//        default:
//            return nullptr;
//            break;
//        }
//    }
//
//
//}
//
