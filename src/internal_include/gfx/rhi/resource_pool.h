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
  T* GetOrCreate(const TInitializer& initializer) {
    const size_t hash = initializer.GetHash();
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
      newResource->Initialize();
      m_pool_[hash] = newResource;
      return newResource;
    }
  }

  template <typename TInitializer, typename T1 = T>
  T* GetOrCreateMove(TInitializer&& initializer) {
    const size_t hash = initializer.GetHash();
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
      newResource->Initialize();
      m_pool_[hash] = newResource;
      return newResource;
    }
  }

  template <typename TInitializer, typename T1 = T>
  void Add(TInitializer&& initializer, T1* InResource) {
    ScopeReadLock sr(&m_lock_);
    const size_t  hash = initializer.GetHash();
    assert(hash);
    assert(InResource);
    if (InResource) {
      m_pool_[hash] = InResource;
    }
  }

  void GetAllResource(std::vector<T*>& Out) {
    Out.reserve(m_pool_.size());
    for (auto it : m_pool_) {
      Out.push_back(it.second);
    }
  }

  template <typename TInitializer>
  void Release(const TInitializer& initializer) {
    Release(initializer.GetHash());
  }

  void Release(size_t hash) {
    ScopeReadLock sr(&m_lock_);
    auto          it_find = m_pool_.find(hash);
    if (m_pool_.end() != it_find) {
      m_pool_.erase(it_find);
    }
  }

  void Release() {
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