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
      ScopeReadLock sr(&Lock);
      auto          it_find = Pool.find(hash);
      if (Pool.end() != it_find) {
        return it_find->second;
      }
    }

    {
      ScopeWriteLock sw(&Lock);

      // Try again, to avoid entering creation section simultaneously.
      auto it_find = Pool.find(hash);
      if (Pool.end() != it_find) {
        return it_find->second;
      }

      auto* newResource = new T1(initializer);
      newResource->Initialize();
      Pool[hash] = newResource;
      return newResource;
    }
  }

  template <typename TInitializer, typename T1 = T>
  T* GetOrCreateMove(TInitializer&& initializer) {
    const size_t hash = initializer.GetHash();
    {
      ScopeReadLock sr(&Lock);
      auto          it_find = Pool.find(hash);
      if (Pool.end() != it_find) {
        return it_find->second;
      }
    }

    {
      ScopeWriteLock sw(&Lock);

      // Try again, to avoid entering creation section simultaneously.
      auto it_find = Pool.find(hash);
      if (Pool.end() != it_find) {
        return it_find->second;
      }

      auto* newResource = new T1(std::move(initializer));
      newResource->Initialize();
      Pool[hash] = newResource;
      return newResource;
    }
  }

  template <typename TInitializer, typename T1 = T>
  void Add(TInitializer&& initializer, T1* InResource) {
    ScopeReadLock sr(&Lock);
    const size_t  hash = initializer.GetHash();
    assert(hash);
    assert(InResource);
    if (InResource) {
      Pool[hash] = InResource;
    }
  }

  void GetAllResource(std::vector<T*>& Out) {
    Out.reserve(Pool.size());
    for (auto it : Pool) {
      Out.push_back(it.second);
    }
  }

  template <typename TInitializer>
  void Release(const TInitializer& initializer) {
    Release(initializer.GetHash());
  }

  void Release(size_t InHash) {
    ScopeReadLock sr(&Lock);
    auto          it_find = Pool.find(InHash);
    if (Pool.end() != it_find) {
      Pool.erase(it_find);
    }
  }

  void Release() {
    ScopeWriteLock sw(&Lock);
    for (auto& iter : Pool) {
      static_assert(sizeof(T) > 0, "Cannot delete pointer of incomplete type");
      delete iter.second;
    }
    Pool.clear();
  }

  std::unordered_map<size_t, T*> Pool;
  LOCK_TYPE                      Lock;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_RESOURCE_POOL_H