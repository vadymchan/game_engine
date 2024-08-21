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
  void lock() { m_lock_.lock(); }

  void unlock() { m_lock_.unlock(); }

  std::mutex m_lock_;
};

// class RWLock {
//   public:
//   void lockRead() {}
//
//   void unlockRead() {}
//
//   void lockWrite() {}
//
//   void unlockWrite() {}
// };
//
// using EmtpyRWLock = RWLock;

class MutexRWLock {
  public:
  void lockRead() { m_lock_.lock_shared(); }

  void unlockRead() { m_lock_.unlock_shared(); }

  void lockWrite() { m_lock_.lock(); }

  void unlockWrite() { m_lock_.unlock(); }

  std::shared_mutex m_lock_;
};

template <typename T>
class ScopedLock {
  public:
  ScopedLock(T* lock)
      : m_scopedLock_(lock) {
    m_scopedLock_->lock();
  }

  ~ScopedLock() { m_scopedLock_->unlock(); }

  T* m_scopedLock_;
};

template <typename T>
class ScopeReadLock {
  public:
  ScopeReadLock(T* lock)
      : m_scopedLock_(lock) {
    m_scopedLock_->lockRead();
  }

  ~ScopeReadLock() { m_scopedLock_->unlockRead(); }

  T* m_scopedLock_;
};

template <typename T>
class ScopeWriteLock {
  public:
  ScopeWriteLock(T* lock)
      : m_scopedLock_(lock) {
    m_scopedLock_->lockWrite();
  }

  ~ScopeWriteLock() { m_scopedLock_->unlockWrite(); }

  T* m_scopedLock_;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_LOCK_H