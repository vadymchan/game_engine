#ifndef GAME_ENGINE_NAME_H
#define GAME_ENGINE_NAME_H

#include "gfx/rhi/lock.h"
#include "utils/third_party/xxhash_util.h"

#include <cassert>
#include <memory>
#include <string>
#include <unordered_map>

// TODO: move file to other folder (e.g. utils)

namespace game_engine {

struct Name {
  private:
  static std::unordered_map<uint32_t, std::shared_ptr<std::string>> s_NameTable;
  static MutexRWLock                                                Lock;

  public:
  static const Name Invalid;

  static uint32_t GenerateNameHash(const char* pName, size_t size) {
    return XXH64(pName, size);
  }

  Name() = default;

  explicit Name(uint32_t InNameHash) {
    NameHash         = InNameHash;
    NameString       = nullptr;
    NameStringLength = 0;
  }

  explicit Name(const char* pName) { Set(pName, strlen(pName)); }

  explicit Name(const char* pName, size_t size) { Set(pName, size); }

  explicit Name(const std::string& name) { Set(name.c_str(), name.length()); }

  Name(const Name& name) { *this = name; }

  void Set(const char* pName, size_t size) {
    assert(pName);
    const uint32_t NewNameHash = GenerateNameHash(pName, size);

    auto find_func = [&]() {
      const auto find_it = s_NameTable.find(NewNameHash);
      if (s_NameTable.end() != find_it) {
        NameHash         = NewNameHash;
        NameString       = find_it->second->c_str();
        NameStringLength = find_it->second->size();
        return true;
      }
      return false;
    };

    {
      ScopeReadLock sr(&Lock);
      if (find_func()) {
        return;
      }
    }

    {
      ScopeWriteLock sw(&Lock);

      // Try again, to avoid entering creation section simultaneously.
      if (find_func()) {
        return;
      }

      const auto it_ret = s_NameTable.emplace(
          NewNameHash, CreateNewName_Internal(pName, NewNameHash));
      if (it_ret.second) {
        NameHash         = NewNameHash;
        NameString       = it_ret.first->second->c_str();
        NameStringLength = it_ret.first->second->size();
        return;
      }
    }

    assert(0);
  }

  operator uint32_t() const {
    assert(NameHash != -1);
    return NameHash;
  }

  Name& operator=(const Name& In) {
    NameHash         = In.NameHash;
    NameString       = In.NameString;
    NameStringLength = In.NameStringLength;
    return *this;
  }

  bool operator==(const Name& rhs) const {
    return GetNameHash() == rhs.GetNameHash();
  }

  bool IsValid() const { return NameHash != -1; }

  const char* ToStr() const {
    if (!IsValid()) {
      return nullptr;
    }

    if (NameString) {
      return NameString;
    }

    {
      ScopeReadLock s(&Lock);
      const auto    it_find = s_NameTable.find(NameHash);
      if (it_find == s_NameTable.end()) {
        return nullptr;
      }

      NameString       = it_find->second->c_str();
      NameStringLength = it_find->second->size();

      return it_find->second->c_str();
    }
  }

  const size_t GetStringLength() const {
    if (!IsValid()) {
      return 0;
    }

    if (!NameStringLength) {
      return NameStringLength;
    }

    {
      ScopeReadLock s(&Lock);
      const auto    it_find = s_NameTable.find(NameHash);
      if (it_find == s_NameTable.end()) {
        return 0;
      }

      NameString       = it_find->second->c_str();
      NameStringLength = it_find->second->size();

      return NameStringLength;
    }
  }

  uint32_t GetNameHash() const { return NameHash; }

  private:
  static std::shared_ptr<std::string> CreateNewName_Internal(
      const char* pName, uint32_t NameHash) {
    assert(pName);

    return std::make_shared<std::string>(pName);
  }

  uint32_t            NameHash         = -1;
  mutable const char* NameString       = nullptr;
  mutable size_t      NameStringLength = 0;
};

struct NameHashFunc {
  std::size_t operator()(const Name& name) const {
    return static_cast<size_t>(name.GetNameHash());
  }
};

struct PriorityName : public Name {
  PriorityName() = default;

  explicit PriorityName(uint32_t InNameHash, uint32_t InPriority)
      : Name(InNameHash)
      , Priority(InPriority) {}

  explicit PriorityName(const char* pName, uint32_t InPriority)
      : Name(pName)
      , Priority(InPriority) {}

  explicit PriorityName(const char* pName, size_t size, uint32_t InPriority)
      : Name(pName, size)
      , Priority(InPriority) {}

  explicit PriorityName(const std::string& name, uint32_t InPriority)
      : Name(name)
      , Priority(InPriority) {}

  explicit PriorityName(const Name& name, uint32_t InPriority)
      : Name(name)
      , Priority(InPriority) {}

  uint32_t Priority = 0;
};

struct PriorityNameHashFunc {
  std::size_t operator()(const PriorityName& name) const {
    return name.GetNameHash();
  }
};

struct PriorityNameComapreFunc {
  bool operator()(const PriorityName& lhs, const PriorityName& rhs) const {
    return lhs.Priority < rhs.Priority;
  }
};

//struct Name {
//  private:
//  static std::unordered_map<uint32_t, std::string> s_NameTable;
//  static MutexRWLock                               Lock;
//
//  public:
//  static const Name Invalid;
//
//  static uint32_t GenerateNameHash(const std::string& name) {
//    // TODO: implement logic (xxHash)
//    return XXH64(name, name.size());
//  }
//
//  Name() = default;
//
//  explicit Name(uint32_t InNameHash) {
//    NameHash   = InNameHash;
//    NameString = "";
//  }
//
//  explicit Name(const std::string& name) { Set(name); }
//
//  Name(const Name& name) { *this = name; }
//
//  void Set(const std::string& name) {
//    assert(!name.empty());
//    const uint32_t NewNameHash = GenerateNameHash(name);
//
//    auto find_func = [&]() {
//      const auto find_it = s_NameTable.find(NewNameHash);
//      if (s_NameTable.end() != find_it) {
//        NameHash   = NewNameHash;
//        NameString = find_it->second;
//        return true;
//      }
//      return false;
//    };
//
//    {
//      ScopeReadLock sr(&Lock);
//      if (find_func()) {
//        return;
//      }
//    }
//
//    {
//      ScopeWriteLock sw(&Lock);
//
//      // Try again, to avoid entering creation section simultaneously.
//      if (find_func()) {
//        return;
//      }
//
//      const auto it_ret = s_NameTable.emplace(NewNameHash, name);
//      if (it_ret.second) {
//        NameHash   = NewNameHash;
//        NameString = it_ret.first->second;
//        return;
//      }
//    }
//
//    assert(0);
//  }
//
//  operator uint32_t() const {
//    assert(NameHash != -1);
//    return NameHash;
//  }
//
//  Name& operator=(const Name& In) {
//    NameHash   = In.NameHash;
//    NameString = In.NameString;
//    return *this;
//  }
//
//  bool operator==(const Name& rhs) const {
//    return GetNameHash() == rhs.GetNameHash();
//  }
//
//  bool IsValid() const { return NameHash != -1; }
//
//  const std::string& ToStr() const {
//    if (!IsValid()) {
//      static const std::string EmptyString;
//      return EmptyString;
//    }
//
//    if (!NameString.empty()) {
//      return NameString;
//    }
//
//    {
//      ScopeReadLock s(&Lock);
//      const auto    it_find = s_NameTable.find(NameHash);
//      if (it_find == s_NameTable.end()) {
//        static const std::string EmptyString;
//        return EmptyString;
//      }
//
//      NameString = it_find->second;
//
//      return it_find->second;
//    }
//  }
//
//  size_t GetStringLength() const {
//    if (!IsValid()) {
//      return 0;
//    }
//
//    if (!NameString.empty()) {
//      return NameString.size();
//    }
//
//    {
//      ScopeReadLock s(&Lock);
//      const auto    it_find = s_NameTable.find(NameHash);
//      if (it_find == s_NameTable.end()) {
//        return 0;
//      }
//
//      NameString = it_find->second;
//
//      return NameString.size();
//    }
//  }
//
//  uint32_t GetNameHash() const { return NameHash; }
//
//  private:
//  uint32_t            NameHash;
//  mutable std::string NameString;
//};
//
//struct NameHashFunc {
//  std::size_t operator()(const Name& name) const {
//    return static_cast<size_t>(name.GetNameHash());
//  }
//};
//
//struct PriorityName : public Name {
//  PriorityName() = default;
//
//  explicit PriorityName(uint32_t InNameHash, uint32_t InPriority)
//      : Name(InNameHash)
//      , Priority(InPriority) {}
//
//  explicit PriorityName(const std::string& name, uint32_t InPriority)
//      : Name(name)
//      , Priority(InPriority) {}
//
//  explicit PriorityName(const Name& name, uint32_t InPriority)
//      : Name(name)
//      , Priority(InPriority) {}
//
//  uint32_t Priority = 0;
//};
//
//struct PriorityNameHashFunc {
//  std::size_t operator()(const PriorityName& name) const {
//    return name.GetNameHash();
//  }
//};
//
//struct PriorityNameCompareFunc {
//  bool operator()(const PriorityName& lhs, const PriorityName& rhs) const {
//    return lhs.Priority < rhs.Priority;
//  }
//};

// TODO: remove this implementation
#define NameStatic(STRING)    \
  [&]() {                     \
    static Name name(STRING); \
    return name;              \
  }()

// template <typename T>
// inline const Name& NameStatic(const T& str) {
//   static Name name(str);
//   return name;
// }

}  // namespace game_engine

#endif GAME_ENGINE_NAME_H