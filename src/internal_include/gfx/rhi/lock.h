#ifndef GAME_ENGINE_LOCK_H
#define GAME_ENGINE_LOCK_H

// TODO: move file to other folder (e.g. utils)

#include <shared_mutex>

namespace game_engine {

// TODO: consider remove Lock and RWLock classes

// class Lock {
//   public:
//   void lock() {}
//
//   void unlock() {}
// };
//
// using EmptyLock = Lock;

class MutexLock {
  public:
  void Lock() { lock.lock(); }

  void Unlock() { lock.unlock(); }

  std::mutex lock;
};

// class RWLock {
//   public:
//   void LockRead() {}
//
//   void UnlockRead() {}
//
//   void LockWrite() {}
//
//   void UnlockWrite() {}
// };
//
// using EmtpyRWLock = RWLock;

class MutexRWLock {
  public:
  void LockRead() { lock.lock_shared(); }

  void UnlockRead() { lock.unlock_shared(); }

  void LockWrite() { lock.lock(); }

  void UnlockWrite() { lock.unlock(); }

  std::shared_mutex lock;
};

template <typename T>
class ScopedLock {
  public:
  ScopedLock(T* InLock)
      : scopedLock(InLock) {
    scopedLock->Lock();
  }

  ~ScopedLock() { scopedLock->Unlock(); }

  T* scopedLock;
};

template <typename T>
class ScopeReadLock {
  public:
  ScopeReadLock(T* InLock)
      : scopedLock(InLock) {
    scopedLock->LockRead();
  }

  ~ScopeReadLock() { scopedLock->UnlockRead(); }

  T* scopedLock;
};

template <typename T>
class ScopeWriteLock {
  public:
  ScopeWriteLock(T* InLock)
      : scopedLock(InLock) {
    scopedLock->LockWrite();
  }

  ~ScopeWriteLock() { scopedLock->UnlockWrite(); }

  T* scopedLock;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_LOCK_H