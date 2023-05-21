
#include "../window/i_window.h"



namespace GameEngine {

    template <typename DerivedRenderer>
    class IRenderer {
    public:
        bool Initialize(IWindow<DerivedRenderer>& window) {
            return static_cast<DerivedRenderer*>(this)->InitializeImplementation(window);
        }

        void Shutdown() {
            return static_cast<DerivedRenderer*>(this)->ShutdownImplementation();
        }

        void BeginFrame() {
            return static_cast<DerivedRenderer*>(this)->BeginFrameImplementation();
        }

        void EndFrame() {
            return static_cast<DerivedRenderer*>(this)->EndFrameImplementation();
        }

        // other renderer-related methods...
    };

}  // namespace GameEngine

