#ifndef GAME_ENGINE_PROFILER_COLORS_H
#define GAME_ENGINE_PROFILER_COLORS_H

#include <cstdint>

namespace game_engine {
namespace profiler {

struct Color {
  float r, g, b, a;

  constexpr Color(float r, float g, float b, float a = 1.0f)
      : r(r)
      , g(g)
      , b(b)
      , a(a) {}

  // (0xRRGGBB)
  static constexpr Color fromHex(uint32_t hex) {
    return Color(((hex >> 16) & 0xFF) / 255.0f,  // R
                 ((hex >> 8) & 0xFF) / 255.0f,   // G
                 ((hex >> 0) & 0xFF) / 255.0f,   // B
                 1.0f                            // A
    );
  }
};

namespace colors {
constexpr Color RENDERING    = Color(0.0f, 1.0f, 0.0f);        // Green
// TODO: other colors
// Color(0.25f, 0.25f, 0.25f);     // Dark gray
// Color(0.0f, 0.0f, 1.0f);        // Blue
// Color(1.0f, 0.0f, 0.0f);        // Red
// Color(1.0f, 1.0f, 0.0f);        // Yellow
// Color(1.0f, 0.0f, 1.0f);        // Magenta
// Color(0.0f, 1.0f, 1.0f);        // Cyan
// Color(1.0f, 0.65f, 0.0f);       // Orange
// Color(0.5f, 0.0f, 0.5f);        // Purple
// Color(0.5f, 0.5f, 1.0f, 0.7f);  // Semi-transparent cyan
}  // namespace colors

}  // namespace profiler
}  // namespace game_engine

#endif