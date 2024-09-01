#ifndef GAME_ENGINE_INSTANT_STRUCT_H
#define GAME_ENGINE_INSTANT_STRUCT_H

#include "utils/third_party/xxhash_util.h"

#include <cstdint>
#include <type_traits>

namespace game_engine {

// TODO: move file to other folder (e.g. utils)

// Struct could be added padding by compiler and it is filled with trash data.
// so, packing rule set to 1 here. If this code removed, you can see the
// different hash with same struct data.
#pragma pack(push, 1)

// InstantStructInternal
// Recursive declaration of variables that passed into variadic template
// arguments
template <typename... Args>
struct InstantStructInternal {};

template <typename T, typename... Args>
struct InstantStructInternal<T, Args...>
    : public InstantStructInternal<Args...> {
  public:
  // TODO: currently uncommented, but may cause inconsistend behavior
  // static_assert(std::is_trivially_copyable<T>::value,
  //               "InstanceStruct members should be 'trivially copyable'");
  static_assert(!std::is_pointer<T>::value,
                "InstanceStruct members should not be 'pointer'");
  // ======= BEGIN: public constructors =======================================

  InstantStructInternal<T, Args...>(T v, Args... args)
      : m_data_(v)
      , InstantStructInternal<Args...>(args...) {}

  // ======= END: public constructors   =======================================

 // ======= BEGIN: public misc fields ========================================

  T m_data_{};

  // ======= END: public misc fields   ========================================



};

template <>
struct InstantStructInternal<> {
  public:
};

// InstantStruct
// A template structure that dynamically generates a struct from the arguments
// provided to its constructor. This structure aims to create a struct directly
// from the variables passed to the constructor. It inherits from
// InstantStructInternal, forwarding the constructor arguments to the base
// class constructor.
template <typename... Args>
struct InstantStruct : public InstantStructInternal<Args...> {
  // ======= BEGIN: public constructors =======================================

  InstantStruct(Args... args)
      : InstantStructInternal<Args...>(args...) {}

  // ======= END: public constructors   =======================================
};

// TODO: check whether this is portable
#pragma pack(pop)

// Easy hash generation from InstantStruct, it is slow calling 'Hash Generation
// Function' for each variables Indivisually.
#define GETHASH_FROM_INSTANT_STRUCT(...)                        \
  [&]() -> uint64_t {                                           \
    auto InstanceStruct = InstantStruct(__VA_ARGS__);           \
    return ::XXH64(&InstanceStruct, sizeof(InstanceStruct), 0); \
  }()

}  // namespace game_engine

#endif  // GAME_ENGINE_INSTANT_STRUCT_H