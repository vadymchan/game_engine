#pragma once

#include "i_renderer.h"
#include "../window/winapi_window.h"

namespace GameEngine {

    class DX11Renderer : public IRenderer<DX11Renderer> {
    public:
        DX11Renderer();
        ~DX11Renderer();

        bool InitializeImplementation(WinApiWindow& window);
        void ShutdownImplementation();

        void BeginFrameImplementation();
        void EndFrameImplementation();

        // other renderer-related methods...
    };

}  // namespace GameEngine
