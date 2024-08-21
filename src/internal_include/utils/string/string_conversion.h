#ifndef GAME_ENGINE_STRING_CONVERSION_H
#define GAME_ENGINE_STRING_CONVERSION_H

#include "gfx/rhi/name.h"

#include <cassert>
#include <string>

namespace game_engine {

// Converts a multi-byte character string to a wide character string
std::wstring g_convertToWchar(const char* path, int32_t length);

// Overloaded function to convert jName type to wide character string
inline std::wstring g_convertToWchar(Name name) {
  assert(name.isValid());
  return g_convertToWchar(name.toStr(), (int32_t)name.getStringLength());
}

}  // namespace game_engine

#endif  // GAME_ENGINE_STRING_CONVERSION_H