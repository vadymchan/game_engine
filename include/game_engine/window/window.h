#pragma once

#include "error_codes.h"
#include <string>

namespace game_engine {
namespace window {

template <typename T>
class Window {
public:
    Window(int width, int height, const std::string& title) 
        : width_(width), height_(height), title_(title) {}

    utils::ErrorCode Initialize() { return static_cast<T*>(this)->InitializeImpl(); }
    bool ShouldClose() const { return static_cast<const T*>(this)->ShouldCloseImpl(); }
    void PollEvents() { static_cast<T*>(this)->PollEventsImpl(); }

protected:
    int width_;
    int height_;
    std::string title_;
};

} // namespace window
} // namespace game_engine
