#pragma once

namespace game_engine {
namespace utils {

enum class ErrorCode {
    Success = 0,
    GLFWInitFailed,
    GLFWWindowCreationFailed,
};

} // namespace utils
} // namespace game_engine
