#pragma once

namespace game_engine {
namespace renderer {

template <typename T>
class RenderAPI {
public:
    void Initialize() { static_cast<T*>(this)->Initialize(); }
    void DrawTriangle() { static_cast<T*>(this)->DrawTriangle(); }
    void Present() { static_cast<T*>(this)->Present(); }
};

} // namespace renderer
} // namespace game_engine
