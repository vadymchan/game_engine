#ifndef GAME_ENGINE_THIRD_PARTY_UTIL_H
#define GAME_ENGINE_THIRD_PARTY_UTIL_H

#include <xxhash.h>

#include <type_traits>

namespace game_engine {

// TODO: consider renaming according to naming conventions
template <typename T>
uint64_t XXH64(const T& data, uint64_t seed = 0) {
  static_assert(std::is_trivially_copyable<T>::value,
                "Custom XXH64 function should be trivially copyable.");
  return ::XXH64(&data, sizeof(T), seed);
}

}  // namespace game_engine

#endif  // GAME_ENGINE_THIRD_PARTY_UTIL_H