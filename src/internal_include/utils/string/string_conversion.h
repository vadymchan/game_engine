#ifndef GAME_ENGINE_STRING_CONVERSION_H
#define GAME_ENGINE_STRING_CONVERSION_H

#include "gfx/rhi/name.h"

#include <cassert>
#include <string>

namespace game_engine {

// Converts a multi-byte character string to a wide character string
std::wstring ConvertToWchar(const char* path, int32_t length);

// Overloaded function to convert jName type to wide character string
inline std::wstring ConvertToWchar(Name name) {
  assert(name.IsValid());
  return ConvertToWchar(name.ToStr(), (int32_t)name.GetStringLength());
}

}  // namespace game_engine

#endif  // GAME_ENGINE_STRING_CONVERSION_H