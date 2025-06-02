#ifndef ARISE_UTILS_COLOR_H
#define ARISE_UTILS_COLOR_H

#include <array>
#include <cstdint>

namespace arise {
namespace color {

// Predefined colors in RGBA format (0xRRGGBBAA)

// Basic colors
constexpr uint32_t RED   = 0xFF'00'00'FF;
constexpr uint32_t GREEN = 0x00'FF'00'FF;
constexpr uint32_t BLUE  = 0x00'00'FF'FF;
constexpr uint32_t WHITE = 0xFF'FF'FF'FF;
constexpr uint32_t BLACK = 0x00'00'00'FF;

// Extended colors
constexpr uint32_t YELLOW  = 0xFF'FF'00'FF;
constexpr uint32_t MAGENTA = 0xFF'00'FF'FF;
constexpr uint32_t CYAN    = 0x00'FF'FF'FF;
constexpr uint32_t ORANGE  = 0xFF'A6'00'FF;
constexpr uint32_t PURPLE  = 0x80'00'80'FF;
constexpr uint32_t BROWN   = 0x8B'45'13'FF;

// Gray variations
constexpr uint32_t DARK_GRAY  = 0x40'40'40'FF;
constexpr uint32_t GRAY       = 0x80'80'80'FF;
constexpr uint32_t LIGHT_GRAY = 0xC0'C0'C0'FF;

// Semi-transparent colors
constexpr uint32_t CYAN_SEMI = 0x80'80'FF'B3;

/**
 * @brief Convert color value to normalized float array
 * @param color Color value in 0xRRGGBBAA format
 * @return Array of 4 floats [R, G, B, A] in range [0.0, 1.0]
 *
 * Works both at compile-time and runtime. Use with color constants
 * or runtime values.
 *
 * Example usage:
 * @code
 * // Compile-time usage
 * constexpr auto red = g_toFloatArray(colors::RED);
 *
 * // Runtime usage
 * uint32_t customColor = 0xFF0080FF;
 * auto colorArray = g_toFloatArray(customColor);
 * @endcode
 */
[[nodiscard]] constexpr std::array<float, 4> g_toFloatArray(uint32_t color) noexcept {
  constexpr float inv255 = 1.0f / 255.0f;
  // clang-format off
  return {
    static_cast<float>((color >> 24) & 0xFF) * inv255,  // R
    static_cast<float>((color >> 16) & 0xFF) * inv255,  // G
    static_cast<float>((color >>  8) & 0xFF) * inv255,  // B
    static_cast<float>((color      ) & 0xFF) * inv255   // A
    };
  // clang-format on
}

}  // namespace color
}  // namespace arise

#endif  // ARISE_UTILS_COLOR_H