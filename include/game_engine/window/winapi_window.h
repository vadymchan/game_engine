#pragma once

#include "i_window.h"
#include <Windows.h>

namespace GameEngine {

    class WinApiWindow : public IWindow<WinApiWindow> {
    public:
        WinApiWindow();
        ~WinApiWindow();

        bool InitializeImplementation(int width, int height, const std::string& title);
        void ShutdownImplementation();

        bool ShouldCloseImplementation() const;
        void PollEventsImplementation() const;
        void SwapBuffersImplementation() const;

        // other window-related methods...

    private:
        HWND hWnd; // Handle to this window
        // Other data members as needed...
    };

}  // namespace GameEngine

