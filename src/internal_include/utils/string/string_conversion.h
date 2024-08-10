#ifndef GAME_ENGINE_STRING_CONVERSION_H
#define GAME_ENGINE_STRING_CONVERSION_H

#include "gfx/rhi/name.h"

#include <cassert>
#include <string>

namespace game_engine {

// Converts a multi-byte character string to a wide character string
std::wstring ConvertToWchar(const char* InPath, int32_t InLength);

// Overloaded function to convert jName type to wide character string
inline std::wstring ConvertToWchar(Name InName) {
  assert(InName.IsValid());
  return ConvertToWchar(InName.ToStr(), (int32_t)InName.GetStringLength());
}

}  // namespace game_engine

#endif  // GAME_ENGINE_STRING_CONVERSION_H