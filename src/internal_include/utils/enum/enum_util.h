#ifndef GAME_ENGINE_ENUM_UTIL_H
#define GAME_ENGINE_ENUM_UTIL_H

#include <unordered_map>

namespace game_engine {

#define DECLARE_ENUM_BIT_OPERATORS(ENUM_TYPE)                                    \
  inline constexpr ENUM_TYPE operator|(ENUM_TYPE lhs, ENUM_TYPE rhs) {           \
    using T = std::underlying_type<ENUM_TYPE>::type;                             \
    return static_cast<ENUM_TYPE>(static_cast<T>(lhs) | static_cast<T>(rhs));    \
  }                                                                              \
  inline constexpr ENUM_TYPE operator&(ENUM_TYPE lhs, ENUM_TYPE rhs) {           \
    using T = std::underlying_type<ENUM_TYPE>::type;                             \
    return static_cast<ENUM_TYPE>(static_cast<T>(lhs) & static_cast<T>(rhs));    \
  }                                                                              \
  inline constexpr ENUM_TYPE operator^(ENUM_TYPE lhs, ENUM_TYPE rhs) {           \
    using T = std::underlying_type<ENUM_TYPE>::type;                             \
    return static_cast<ENUM_TYPE>(static_cast<T>(lhs) ^ static_cast<T>(rhs));    \
  }                                                                              \
  inline constexpr ENUM_TYPE& operator|=(ENUM_TYPE& lhs, ENUM_TYPE rhs) {        \
    using T = std::underlying_type<ENUM_TYPE>::type;                             \
    lhs     = static_cast<ENUM_TYPE>(static_cast<T>(lhs) | static_cast<T>(rhs)); \
    return lhs;                                                                  \
  }                                                                              \
  inline constexpr ENUM_TYPE& operator&=(ENUM_TYPE& lhs, ENUM_TYPE rhs) {        \
    using T = std::underlying_type<ENUM_TYPE>::type;                             \
    lhs     = static_cast<ENUM_TYPE>(static_cast<T>(lhs) & static_cast<T>(rhs)); \
    return lhs;                                                                  \
  }                                                                              \
  inline constexpr ENUM_TYPE& operator^=(ENUM_TYPE& lhs, ENUM_TYPE rhs) {        \
    using T = std::underlying_type<ENUM_TYPE>::type;                             \
    lhs     = static_cast<ENUM_TYPE>(static_cast<T>(lhs) ^ static_cast<T>(rhs)); \
    return lhs;                                                                  \
  }                                                                              \
  inline constexpr bool operator!(ENUM_TYPE value) {                             \
    using T = std::underlying_type<ENUM_TYPE>::type;                             \
    return !static_cast<T>(value);                                               \
  }                                                                              \
  inline constexpr ENUM_TYPE operator~(ENUM_TYPE value) {                        \
    using T = std::underlying_type<ENUM_TYPE>::type;                             \
    return static_cast<ENUM_TYPE>(~static_cast<T>(value));                       \
  }

template <typename Key, typename Value>
Value getEnumMapping(const std::unordered_map<Key, Value>& mapping,
                     const Key&                            key,
                     const Value&                          defaultValue) {
  auto it = mapping.find(key);
  if (it != mapping.end()) {
    return it->second;
  }
  return defaultValue;
}

template <typename Key, typename Value>
std::unordered_map<Value, Key> reverseMap(
    const std::unordered_map<Key, Value>& map) {
  std::unordered_map<Value, Key> reversedMap;
  for (const auto& [key, value] : map) {
    reversedMap[value] = key;
  }
  return reversedMap;
}

}  // namespace game_engine
#endif  // GAME_ENGINE_ENUM_UTIL_H
