#ifndef GAME_ENGINE_THIRD_PARTY_UTIL_H
#define GAME_ENGINE_THIRD_PARTY_UTIL_H

#include <xxhash.h>

#include <type_traits>

namespace game_engine {

template <typename T>
uint64_t XXH64(const T& InData, uint64_t InSeed = 0) {
  static_assert(std::is_trivially_copyable<T>::value,
                "Custom XXH64 function should be trivially copyable.");
  // TODO: remove
  //static_assert(!std::is_pointer<T>::value,
  //              "Custom XXH64 function is not allowed pointer type.");
  return ::XXH64(&InData, sizeof(T), InSeed);
}

}  // namespace game_engine

#endif  // GAME_ENGINE_THIRD_PARTY_UTIL_H