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
  void Lock() { m_lock_.lock(); }

  void Unlock() { m_lock_.unlock(); }

  std::mutex m_lock_;
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
  void LockRead() { m_lock_.lock_shared(); }

  void UnlockRead() { m_lock_.unlock_shared(); }

  void LockWrite() { m_lock_.lock(); }

  void UnlockWrite() { m_lock_.unlock(); }

  std::shared_mutex m_lock_;
};

template <typename T>
class ScopedLock {
  public:
  ScopedLock(T* InLock)
      : m_scopedLock_(InLock) {
    m_scopedLock_->Lock();
  }

  ~ScopedLock() { m_scopedLock_->Unlock(); }

  T* m_scopedLock_;
};

template <typename T>
class ScopeReadLock {
  public:
  ScopeReadLock(T* InLock)
      : m_scopedLock_(InLock) {
    m_scopedLock_->LockRead();
  }

  ~ScopeReadLock() { m_scopedLock_->UnlockRead(); }

  T* m_scopedLock_;
};

template <typename T>
class ScopeWriteLock {
  public:
  ScopeWriteLock(T* InLock)
      : m_scopedLock_(InLock) {
    m_scopedLock_->LockWrite();
  }

  ~ScopeWriteLock() { m_scopedLock_->UnlockWrite(); }

  T* m_scopedLock_;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_LOCK_H