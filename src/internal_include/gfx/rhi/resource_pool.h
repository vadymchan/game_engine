#ifndef GAME_ENGINE_RESOURCE_POOL_H
#define GAME_ENGINE_RESOURCE_POOL_H

#include "gfx/rhi/lock.h"

#include <cassert>
#include <unordered_map>

namespace game_engine {

template <typename T, typename LOCK_TYPE>
class TResourcePool {
  public:
  template <typename TInitializer, typename T1 = T>
  T* getOrCreate(const TInitializer& initializer) {
    const size_t hash = initializer.getHash();
    {
      ScopeReadLock sr(&m_lock_);
      auto          it_find = m_pool_.find(hash);
      if (m_pool_.end() != it_find) {
        return it_find->second;
      }
    }

    {
      ScopeWriteLock sw(&m_lock_);

      // Try again, to avoid entering creation section simultaneously.
      auto it_find = m_pool_.find(hash);
      if (m_pool_.end() != it_find) {
        return it_find->second;
      }

      auto* newResource = new T1(initializer);
      newResource->initialize();
      m_pool_[hash] = newResource;
      return newResource;
    }
  }

  template <typename TInitializer, typename T1 = T>
  T* getOrCreateMove(TInitializer&& initializer) {
    const size_t hash = initializer.getHash();
    {
      ScopeReadLock sr(&m_lock_);
      auto          it_find = m_pool_.find(hash);
      if (m_pool_.end() != it_find) {
        return it_find->second;
      }
    }

    {
      ScopeWriteLock sw(&m_lock_);

      // Try again, to avoid entering creation section simultaneously.
      auto it_find = m_pool_.find(hash);
      if (m_pool_.end() != it_find) {
        return it_find->second;
      }

      auto* newResource = new T1(std::move(initializer));
      newResource->initialize();
      m_pool_[hash] = newResource;
      return newResource;
    }
  }

  template <typename TInitializer, typename T1 = T>
  void add(TInitializer&& initializer, T1* resource) {
    ScopeReadLock sr(&m_lock_);
    const size_t  hash = initializer.getHash();
    assert(hash);
    assert(resource);
    if (resource) {
      m_pool_[hash] = resource;
    }
  }

  void getAllResource(std::vector<T*>& Out) {
    Out.reserve(m_pool_.size());
    for (auto it : m_pool_) {
      Out.push_back(it.second);
    }
  }

  template <typename TInitializer>
  void release(const TInitializer& initializer) {
    release(initializer.getHash());
  }

  void release(size_t hash) {
    ScopeReadLock sr(&m_lock_);
    auto          it_find = m_pool_.find(hash);
    if (m_pool_.end() != it_find) {
      m_pool_.erase(it_find);
    }
  }

  void release() {
    ScopeWriteLock sw(&m_lock_);
    for (auto& iter : m_pool_) {
      static_assert(sizeof(T) > 0, "Cannot delete pointer of incomplete type");
      delete iter.second;
    }
    m_pool_.clear();
  }

  std::unordered_map<size_t, T*> m_pool_;
  LOCK_TYPE                      m_lock_;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_RESOURCE_POOL_H