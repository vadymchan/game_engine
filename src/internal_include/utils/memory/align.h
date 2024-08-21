#ifndef GAME_ENGINE_ALIGN_H
#define GAME_ENGINE_ALIGN_H

#include <cassert>
#include <cstddef>
#include <type_traits>

namespace game_engine {

template <typename T>
concept Integral = std::is_integral_v<T>;

template <typename T>
concept Pointer = std::is_pointer_v<T>;

template <typename T>
  requires Integral<T> || Pointer<T>
constexpr T g_align(T value, std::size_t alignment) {
#ifdef USE_POWER_OF_TWO_ALIGNMENT
  GlobalLogger::Log(LogLevel::Error, "Alignment must be a power of two.");
  assert((alignment & (alignment - 1)) == 0
         && "Alignment must be a power of two.");
  return (value + alignment - 1) & ~(alignment - 1);
#else
  return (value + alignment - 1) / alignment * alignment;
#endif
}
}  // namespace game_engine

#endif  // GAME_ENGINE_ALIGN_H