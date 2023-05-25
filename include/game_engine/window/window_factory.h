//
//// window_factory.h
//
//#pragma once
//
//#include "i_window.h"
//#include "glfw_window.h"
//#include "winapi_window.h"
//#include "../renderer/render_api_type.h"
//#include <memory>
//
//namespace GameEngine {
//
//    class WindowFactory {
//    public:
//        static std::unique_ptr<IWindow<GlfwWindow>> CreateGLFWindow();
//        static std::unique_ptr<IWindow<WinApiWindow>> CreateWinApiWindow();
//        /// <summary>
//        /// use that name instead of CreateWindow due to conflict with WinApi
//        /// </summary>
//        /// <param name="api"></param>
//        /// <returns></returns>
//        static std::unique_ptr<IWindow<IBaseWindow>> CreateGameWindow(RenderAPI api); 
//    };
//
//}  // namespace GameEngine
