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
  public:
  // ======= BEGIN: public static methods =====================================

  static uint32_t s_generateNameHash(const char* pName, size_t size) {
    return XXH64(pName, size);
  }

  // ======= END: public static methods =====================================

  // ======= BEGIN: public static fields ======================================

  static const Name s_kInvalid;

  // ======= END: public static fields   ======================================

  // ======= BEGIN: public constructors =======================================

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

  // ======= END: public constructors   =======================================

  // ======= BEGIN: public getters ============================================

  const size_t getStringLength() const {
    if (!isValid()) {
      return 0;
    }

    if (!m_nameStringLength_) {
      return m_nameStringLength_;
    }

    {
      ScopeReadLock s(&s_lock);
      // TODO: consider rename to kIteratorFind
      const auto    kItFind = s_nameTable.find(m_nameHash_);
      if (kItFind == s_nameTable.end()) {
        return 0;
      }

      m_nameString_       = kItFind->second->c_str();
      m_nameStringLength_ = kItFind->second->size();

      return m_nameStringLength_;
    }
  }

  uint32_t getNameHash() const { return m_nameHash_; }

  // ======= END: public getters   ============================================

  // ======= BEGIN: public setters ============================================

  void set(const char* pName, size_t size) {
    assert(pName);
    const uint32_t kNewNameHash = s_generateNameHash(pName, size);

    auto findFunc = [&]() {
      // TODO: consider rename to kFindIterator
      const auto kFindIt = s_nameTable.find(kNewNameHash);
      if (s_nameTable.end() != kFindIt) {
        m_nameHash_         = kNewNameHash;
        m_nameString_       = kFindIt->second->c_str();
        m_nameStringLength_ = kFindIt->second->size();
        return true;
      }
      return false;
    };

    {
      ScopeReadLock sr(&s_lock);
      if (findFunc()) {
        return;
      }
    }

    {
      ScopeWriteLock sw(&s_lock);

      // Try again, to avoid entering creation section simultaneously.
      if (findFunc()) {
        return;
      }

      // TODO: consider rename to more meaningful name like kIteratorReturn
      const auto kItRet = s_nameTable.emplace(
          kNewNameHash, s_createNewNameInternal(pName, kNewNameHash));
      if (kItRet.second) {
        m_nameHash_         = kNewNameHash;
        m_nameString_       = kItRet.first->second->c_str();
        m_nameStringLength_ = kItRet.first->second->size();
        return;
      }
    }

    assert(0);
  }

  // ======= END: public setters   ============================================

  // ======= BEGIN: public misc methods =======================================

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
      // TODO: consider rename to kIteratorFind (+ the code is inconsistent -
      // sometimes it's kItFind, sometimes kFindIt)
      const auto    kItFind = s_nameTable.find(m_nameHash_);
      if (kItFind == s_nameTable.end()) {
        return nullptr;
      }

      m_nameString_       = kItFind->second->c_str();
      m_nameStringLength_ = kItFind->second->size();

      return kItFind->second->c_str();
    }
  }

  // ======= END: public misc methods   =======================================

  // ======= BEGIN: public overloaded operators ===============================

  operator uint32_t() const {
    assert(m_nameHash_ != -1);
    return m_nameHash_;
  }

  Name& operator=(const Name& other) {
    m_nameHash_         = other.m_nameHash_;
    m_nameString_       = other.m_nameString_;
    m_nameStringLength_ = other.m_nameStringLength_;
    return *this;
  }

  bool operator==(const Name& rhs) const {
    return getNameHash() == rhs.getNameHash();
  }

  // ======= END: public overloaded operators   ===============================

  private:
  // ======= BEGIN: private static methods ====================================

  static std::shared_ptr<std::string> s_createNewNameInternal(
      const char* pName, uint32_t NameHash) {
    assert(pName);

    return std::make_shared<std::string>(pName);
  }

  // ======= END: private static methods   ====================================

  // ======= BEGIN: private static fields =====================================

  static std::unordered_map<uint32_t, std::shared_ptr<std::string>> s_nameTable;
  static MutexRWLock                                                s_lock;

  // ======= END: private static fields   =====================================

  // ======= BEGIN: private misc fields =======================================

  uint32_t            m_nameHash_         = -1;
  mutable const char* m_nameString_       = nullptr;
  mutable size_t      m_nameStringLength_ = 0;

  // ======= END: private misc fields   =======================================
};

struct NameHashFunc {
  // ======= BEGIN: public overloaded operators ===============================

  std::size_t operator()(const Name& name) const {
    return static_cast<size_t>(name.getNameHash());
  }

  // ======= END: public overloaded operators   ===============================
};

struct PriorityName : public Name {
  // ======= BEGIN: public constructors =======================================

  PriorityName() = default;

  explicit PriorityName(uint32_t nameHash, uint32_t priority)
      : Name(nameHash)
      , m_priority_(priority) {}

  explicit PriorityName(const char* pName, uint32_t priority)
      : Name(pName)
      , m_priority_(priority) {}

  // TODO: consider change the order of size and priority parameters (currently
  // it's error-prone)
  explicit PriorityName(const char* pName, size_t size, uint32_t priority)
      : Name(pName, size)
      , m_priority_(priority) {}

  explicit PriorityName(const std::string& name, uint32_t priority)
      : Name(name)
      , m_priority_(priority) {}

  explicit PriorityName(const Name& name, uint32_t priority)
      : Name(name)
      , m_priority_(priority) {}

  // ======= END: public constructors   =======================================

  // ======= BEGIN: public misc fields ========================================

  uint32_t m_priority_ = 0;

  // ======= END: public misc fields   ========================================
};

struct PriorityNameHashFunc {
  // ======= BEGIN: public overloaded operators ===============================

  std::size_t operator()(const PriorityName& name) const {
    return name.getNameHash();
  }

  // ======= END: public overloaded operators   ===============================
};

struct PriorityNameComapreFunc {
  // ======= BEGIN: public overloaded operators ===============================

  bool operator()(const PriorityName& lhs, const PriorityName& rhs) const {
    return lhs.m_priority_ < rhs.m_priority_;
  }

  // ======= END: public overloaded operators   ===============================
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