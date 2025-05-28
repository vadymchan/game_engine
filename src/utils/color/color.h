#ifndef GAME_ENGINE_UTILS_COLOR_H
#define GAME_ENGINE_UTILS_COLOR_H

#include <array>
#include <cstdint>

namespace game_engine {
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
  // clang-format off
  return {
    static_cast<float>((color)       & 0xFF) / 255.0f,  // R
    static_cast<float>((color >> 8)  & 0xFF) / 255.0f,  // G
    static_cast<float>((color >> 16) & 0xFF) / 255.0f,  // B
    static_cast<float>((color >> 24) & 0xFF) / 255.0f   // A
  };
  // clang-format on
}


/**
 * @brief Strip alpha channel from RGBA color
 * @param rgba Color value in 0xRRGGBBAA format
 * @return Color value in 0xRRGGBB format (without alpha)
 *
 * This function removes the alpha channel from a color value,
 * effectively converting it to RGB format.
 *
 * Example usage:
 * @code
 * uint32_t colorWithAlpha = 0xFF'00'00'80; // Red with 50% alpha
 * uint32_t rgbColor = g_stripAlpha(colorWithAlpha); // Result: 0xFF'00'00
 * @endcode
 */
[[nodiscard]] constexpr uint32_t g_stripAlpha(uint32_t rgba) noexcept {
  return rgba >> 8;
}

}  // namespace color
}  // namespace game_engine

#endif  // GAME_ENGINE_UTILS_COLOR_H