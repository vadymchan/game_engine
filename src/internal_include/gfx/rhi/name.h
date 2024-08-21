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
  static std::unordered_map<uint32_t, std::shared_ptr<std::string>> s_nameTable;
  static MutexRWLock                                                s_lock;

  public:
  static const Name s_kInvalid;

  static uint32_t s_generateNameHash(const char* pName, size_t size) {
    return XXH64(pName, size);
  }

  Name() = default;

  explicit Name(uint32_t nameHash) {
    m_nameHash_         = nameHash;
    m_nameString_       = nullptr;
    m_nameStringLength_ = 0;
  }

  explicit Name(const char* pName) { set(pName, strlen(pName)); }

  explicit Name(const char* pName, size_t size) { set(pName, size); }

  explicit Name(const std::string& name) { set(name.c_str(), name.length()); }

  Name(const Name& name) { *this = name; }

  void set(const char* pName, size_t size) {
    assert(pName);
    const uint32_t NewNameHash = s_generateNameHash(pName, size);

    auto find_func = [&]() {
      const auto find_it = s_nameTable.find(NewNameHash);
      if (s_nameTable.end() != find_it) {
        m_nameHash_         = NewNameHash;
        m_nameString_       = find_it->second->c_str();
        m_nameStringLength_ = find_it->second->size();
        return true;
      }
      return false;
    };

    {
      ScopeReadLock sr(&s_lock);
      if (find_func()) {
        return;
      }
    }

    {
      ScopeWriteLock sw(&s_lock);

      // Try again, to avoid entering creation section simultaneously.
      if (find_func()) {
        return;
      }

      const auto it_ret = s_nameTable.emplace(
          NewNameHash, s_createNewNameInternal(pName, NewNameHash));
      if (it_ret.second) {
        m_nameHash_         = NewNameHash;
        m_nameString_       = it_ret.first->second->c_str();
        m_nameStringLength_ = it_ret.first->second->size();
        return;
      }
    }

    assert(0);
  }

  operator uint32_t() const {
    assert(m_nameHash_ != -1);
    return m_nameHash_;
  }

  Name& operator=(const Name& In) {
    m_nameHash_         = In.m_nameHash_;
    m_nameString_       = In.m_nameString_;
    m_nameStringLength_ = In.m_nameStringLength_;
    return *this;
  }

  bool operator==(const Name& rhs) const {
    return getNameHash() == rhs.getNameHash();
  }

  bool isValid() const { return m_nameHash_ != -1; }

  const char* toStr() const {
    if (!isValid()) {
      return nullptr;
    }

    if (m_nameString_) {
      return m_nameString_;
    }

    {
      ScopeReadLock s(&s_lock);
      const auto    it_find = s_nameTable.find(m_nameHash_);
      if (it_find == s_nameTable.end()) {
        return nullptr;
      }

      m_nameString_       = it_find->second->c_str();
      m_nameStringLength_ = it_find->second->size();

      return it_find->second->c_str();
    }
  }

  const size_t getStringLength() const {
    if (!isValid()) {
      return 0;
    }

    if (!m_nameStringLength_) {
      return m_nameStringLength_;
    }

    {
      ScopeReadLock s(&s_lock);
      const auto    it_find = s_nameTable.find(m_nameHash_);
      if (it_find == s_nameTable.end()) {
        return 0;
      }

      m_nameString_       = it_find->second->c_str();
      m_nameStringLength_ = it_find->second->size();

      return m_nameStringLength_;
    }
  }

  uint32_t getNameHash() const { return m_nameHash_; }

  private:
  static std::shared_ptr<std::string> s_createNewNameInternal(
      const char* pName, uint32_t NameHash) {
    assert(pName);

    return std::make_shared<std::string>(pName);
  }

  uint32_t            m_nameHash_         = -1;
  mutable const char* m_nameString_       = nullptr;
  mutable size_t      m_nameStringLength_ = 0;
};

struct NameHashFunc {
  std::size_t operator()(const Name& name) const {
    return static_cast<size_t>(name.getNameHash());
  }
};

struct PriorityName : public Name {
  PriorityName() = default;

  explicit PriorityName(uint32_t nameHash, uint32_t priority)
      : Name(nameHash)
      , m_priority_(priority) {}

  explicit PriorityName(const char* pName, uint32_t priority)
      : Name(pName)
      , m_priority_(priority) {}

  // TODO: consider change the order of size and priority parameters (currently it's error-prone)
  explicit PriorityName(const char* pName, size_t size, uint32_t priority)
      : Name(pName, size)
      , m_priority_(priority) {}

  explicit PriorityName(const std::string& name, uint32_t priority)
      : Name(name)
      , m_priority_(priority) {}

  explicit PriorityName(const Name& name, uint32_t priority)
      : Name(name)
      , m_priority_(priority) {}

  uint32_t m_priority_ = 0;
};

struct PriorityNameHashFunc {
  std::size_t operator()(const PriorityName& name) const {
    return name.getNameHash();
  }
};

struct PriorityNameComapreFunc {
  bool operator()(const PriorityName& lhs, const PriorityName& rhs) const {
    return lhs.m_priority_ < rhs.m_priority_;
  }
};

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