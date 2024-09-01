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
  // ======= BEGIN: public misc methods =======================================

  void lock() { m_lock_.lock(); }

  void unlock() { m_lock_.unlock(); }

  // ======= END: public misc methods   =======================================

  // ======= BEGIN: public misc fields ========================================

  std::mutex m_lock_;

  // ======= END: public misc fields   ========================================
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
  // ======= BEGIN: public misc methods =======================================

  void lockRead() { m_lock_.lock_shared(); }

  void unlockRead() { m_lock_.unlock_shared(); }

  void lockWrite() { m_lock_.lock(); }

  void unlockWrite() { m_lock_.unlock(); }

  // ======= END: public misc methods   =======================================

  // ======= BEGIN: public misc fields ========================================

  std::shared_mutex m_lock_;

  // ======= END: public misc fields   ========================================
};

template <typename T>
class ScopedLock {
  public:
  // ======= BEGIN: public constructors =======================================

  ScopedLock(T* lock)
      : m_scopedLock_(lock) {
    m_scopedLock_->lock();
  }

  // ======= END: public constructors   =======================================

  // ======= BEGIN: public destructor =========================================

  ~ScopedLock() { m_scopedLock_->unlock(); }

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public misc fields ========================================

  T* m_scopedLock_;

  // ======= END: public misc fields   ========================================
};

template <typename T>
class ScopeReadLock {
  public:
  // ======= BEGIN: public constructors =======================================

  ScopeReadLock(T* lock)
      : m_scopedLock_(lock) {
    m_scopedLock_->lockRead();
  }

  // ======= END: public constructors   =======================================

  // ======= BEGIN: public destructor =========================================

  ~ScopeReadLock() { m_scopedLock_->unlockRead(); }

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public misc fields ========================================

  T* m_scopedLock_;

  // ======= END: public misc fields   ========================================
};

template <typename T>
class ScopeWriteLock {
  public:
  // ======= BEGIN: public constructors =======================================

  ScopeWriteLock(T* lock)
      : m_scopedLock_(lock) {
    m_scopedLock_->lockWrite();
  }

  // ======= END: public constructors   =======================================

  // ======= BEGIN: public destructor =========================================

  ~ScopeWriteLock() { m_scopedLock_->unlockWrite(); }

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public misc fields ========================================

  T* m_scopedLock_;

  // ======= END: public misc fields   ========================================
};

}  // namespace game_engine

#endif  // GAME_ENGINE_LOCK_H