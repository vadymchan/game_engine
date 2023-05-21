
#include "../window/i_window.h"

namespace GameEngine {

    class IRenderer {
    public:
        virtual ~IRenderer() = default;

        virtual bool Initialize(IWindow& window) = 0;
        virtual void Shutdown() = 0;

        virtual void BeginFrame() = 0;
        virtual void EndFrame() = 0;

        // other renderer-related methods...
    };

}  // namespace GameEngine

