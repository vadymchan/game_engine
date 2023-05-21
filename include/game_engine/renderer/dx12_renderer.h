#ifndef GAME_ENGINE_DX12_RENDERER_H
#define GAME_ENGINE_DX12_RENDERER_H

#include "i_renderer.h"
#include "../window/winapi_window.h"

namespace GameEngine {

    class DX12Renderer : public IRenderer<DX12Renderer> {
    public:
        DX12Renderer();
        ~DX12Renderer();

        bool InitializeImplementation(WinApiWindow& window);
        void ShutdownImplementation();

        void BeginFrameImplementation();
        void EndFrameImplementation();

        // other renderer-related methods...
    };

}  // namespace GameEngine

#endif  // GAME_ENGINE_DX12_RENDERER_H
