#ifndef GAME_ENGINE_MEM_STACK_ALLOCATOR_H
#define GAME_ENGINE_MEM_STACK_ALLOCATOR_H

#define ENABLE_ALLOCATOR_LOG 0
#define DEFAULT_ALIGNMENT    16

#include "gfx/rhi/lock.h"
#include "utils/memory/align.h"

#include <cassert>

namespace game_engine {

struct MemoryChunk {
  // ======= BEGIN: public constructors =======================================

  MemoryChunk() = default;

  MemoryChunk(uint8_t* address, uint64_t offset, uint64_t dataSize)
      : m_address_(address)
      , m_offset_(offset)
      , m_dataSize_(dataSize) {}

  // ======= END: public constructors   =======================================

  // ======= BEGIN: public misc methods =======================================

  void* alloc(size_t numOfBytes) {
    const uint64_t kAllocOffset
        = g_align<uint64_t>(m_offset_, DEFAULT_ALIGNMENT);
    if (kAllocOffset + numOfBytes <= m_dataSize_) {
      m_offset_ = kAllocOffset + numOfBytes;
      return m_address_ + kAllocOffset;
    }
    return nullptr;
  }

  // ======= END: public misc methods   =======================================

  // ======= BEGIN: public misc fields ========================================

  uint8_t* m_address_  = nullptr;
  uint64_t m_offset_   = 0;
  uint64_t m_dataSize_ = 0;

  MemoryChunk* m_next_ = nullptr;

  // ======= END: public misc fields   ========================================
};

class PageAllocator {
  public:
  // ======= BEGIN: public static methods =====================================

  static PageAllocator* get() {
    static PageAllocator* s_pageAllocator = nullptr;
    if (!s_pageAllocator) {
      s_pageAllocator = new PageAllocator();
    }

    return s_pageAllocator;
  }

  // ======= END: public static methods   =====================================

  // ======= BEGIN: public static fields ======================================

  static constexpr uint64_t s_kPageSize = 1024 * 4;
  static constexpr uint64_t s_kMaxMemoryChunkSize
      = s_kPageSize - sizeof(MemoryChunk);

  // ======= END: public static fields   ======================================

  // ======= BEGIN: public misc methods =======================================

  MemoryChunk* allocate() {
    {
      ScopedLock s(&m_lock_);
      if (m_freeChunk_) {
        MemoryChunk* chunk = m_freeChunk_;
        m_freeChunk_       = m_freeChunk_->m_next_;
        return chunk;
      }
    }

    uint8_t*     newLowMemory   = static_cast<uint8_t*>(malloc(s_kPageSize));
    MemoryChunk* newMemoryChunk = new (newLowMemory) MemoryChunk(
        newLowMemory, sizeof(MemoryChunk), PageAllocator::s_kPageSize);
    return newMemoryChunk;
  }

  void free(MemoryChunk* chunk) {
    ScopedLock s(&m_lock_);

    chunk->m_offset_ = sizeof(MemoryChunk);
    chunk->m_next_   = m_freeChunk_;
    m_freeChunk_     = chunk;
  }

  MemoryChunk* allocateBigSize(uint64_t numOfBytes) {
    const uint64_t kNeedToAllocateBytes = numOfBytes + sizeof(MemoryChunk);

    if (m_freeChunkBigSize_) {
      MemoryChunk* chunkPrev = nullptr;
      MemoryChunk* curChunk  = m_freeChunkBigSize_;
      while (curChunk) {
        if (curChunk->m_dataSize_ >= kNeedToAllocateBytes) {
          if (chunkPrev) {
            chunkPrev->m_next_ = curChunk->m_next_;
          }

          return curChunk;
        }

        chunkPrev = curChunk;
        curChunk  = curChunk->m_next_;
      }
    }

    uint8_t* newLowMemory = static_cast<uint8_t*>(malloc(kNeedToAllocateBytes));
    MemoryChunk* newChunk = new (newLowMemory)
        MemoryChunk(newLowMemory, sizeof(MemoryChunk), kNeedToAllocateBytes);
    return newChunk;
  }

  void freeBigSize(MemoryChunk* chunk) {
    chunk->m_offset_ = sizeof(MemoryChunk);

    if (m_freeChunkBigSize_) {
      MemoryChunk* chunkPrev = nullptr;
      MemoryChunk* curChunk  = m_freeChunkBigSize_;
      while (curChunk) {
        if (curChunk->m_dataSize_ >= chunk->m_dataSize_) {
          if (chunkPrev) {
            chunkPrev->m_next_ = chunk;
          }
          chunk->m_next_ = curChunk;
          return;
        }

        chunkPrev = curChunk;
        curChunk  = curChunk->m_next_;
      }

      if (chunkPrev) {
        chunkPrev->m_next_ = chunk;
        return;
      }
    }

    m_freeChunkBigSize_ = chunk;
  }

  void flush() {
    ScopedLock s(&m_lock_);
    while (m_freeChunk_) {
      MemoryChunk* next = m_freeChunk_->m_next_;
      delete[] m_freeChunk_;
      m_freeChunk_ = next;
    }
  }

  // ======= END: public misc methods   =======================================

  // ======= BEGIN: public misc fields ========================================

  MutexLock    m_lock_;
  MemoryChunk* m_freeChunk_        = nullptr;
  MemoryChunk* m_freeChunkBigSize_ = nullptr;

  // ======= END: public misc fields   ========================================

  private:
  // ======= BEGIN: private constructors ======================================

  // TODO: this probably should be public
  PageAllocator()                                = default;
  PageAllocator(const PageAllocator&)            = delete;
  PageAllocator(PageAllocator&&)                 = delete;
  PageAllocator& operator=(const PageAllocator&) = delete;

  // ======= END: private constructors   ======================================
};

class MemStack {
  public:
  // ======= BEGIN: public static methods =====================================

  static MemStack* get() {
    static MemStack* s_memStack = nullptr;
    if (!s_memStack) {
      s_memStack = new MemStack();
    }

    return s_memStack;
  }

  // ======= END: public static methods   =====================================

  // ======= BEGIN: public misc methods =======================================

  template <typename T>
  T* alloc() {
    return (T*)alloc(sizeof(T));
  }

  void* alloc(uint64_t numOfBytes) {
    if (numOfBytes >= PageAllocator::s_kMaxMemoryChunkSize) {
      MemoryChunk* newChunk = PageAllocator::get()->allocateBigSize(numOfBytes);
      assert(newChunk);

      newChunk->m_next_ = m_bigSizeChunk_;
      m_bigSizeChunk_   = newChunk;

      void* allocatedMemory = m_bigSizeChunk_->alloc(numOfBytes);
      assert(allocatedMemory);
      return allocatedMemory;
    }

    if (m_topMemoryChunk_) {
      void* allocatedMemory = m_topMemoryChunk_->alloc(numOfBytes);
      if (allocatedMemory) {
        return allocatedMemory;
      }
    }

    MemoryChunk* newChunk = PageAllocator::get()->allocate();
    assert(newChunk);

    newChunk->m_next_ = m_topMemoryChunk_;
    m_topMemoryChunk_ = newChunk;

    void* allocatedMemory = m_topMemoryChunk_->alloc(numOfBytes);
    assert(allocatedMemory);
    return allocatedMemory;
  }

  void flush() {
    while (m_topMemoryChunk_) {
      MemoryChunk* next = m_topMemoryChunk_->m_next_;
      PageAllocator::get()->free(m_topMemoryChunk_);
      m_topMemoryChunk_ = next;
    }

    while (m_bigSizeChunk_) {
      MemoryChunk* next = m_bigSizeChunk_->m_next_;
      PageAllocator::get()->freeBigSize(m_bigSizeChunk_);
      m_bigSizeChunk_ = next;
    }
  }

  // ======= END: public misc methods   =======================================

  private:
  // ======= BEGIN: private misc fields =======================================

  MemoryChunk* m_topMemoryChunk_ = nullptr;
  MemoryChunk* m_bigSizeChunk_   = nullptr;

  // ======= END: private misc fields   =======================================
};

template <typename T>
class MemStackAllocator {
  public:
  // ======= BEGIN: public aliases ============================================

  // TODO: rewrite to using instead of typedef
  typedef size_t    size_type;
  typedef ptrdiff_t difference_type;
  typedef T*        pointer;
  typedef const T*  const_pointer;
  typedef T&        reference;
  typedef const T&  const_reference;
  typedef T         value_type;

  // ======= END: public aliases   ============================================

  // ======= BEGIN: public misc methods =======================================

  // TODO: refactor numOfElements
  // TODO: for all ENABLE_ALLOCATOR_LOG use logger class (GlobalLogger)
  pointer allocate(size_t numOfElement) {
#if ENABLE_ALLOCATOR_LOG
    pointer allocatedAddress = static_cast<pointer>(
        MemStack::get()->alloc(numOfElement * sizeof(T)));
    std::cout << "Called MemstackAllocator::allocate with 0x"
              << allocatedAddress << " address and " << numOfElement
              << " elements" << std::endl;
    return allocatedAddress;
#else
    return static_cast<pointer>(
        MemStack::get()->alloc(numOfElement * sizeof(T)));
#endif
  }

  void deallocate(pointer address, size_t numOfElement) {
#if ENABLE_ALLOCATOR_LOG
    std::cout << "Called MemstackAllocator::deallocate with 0x" << address
              << " address and " << numOfElement << " elements" << std::endl;
#endif
    // free(address);
  }

  void construct(pointer address, const const_reference initialValue) {
#if ENABLE_ALLOCATOR_LOG
    std::cout << "Called MemstackAllocator::construct with 0x" << address
              << " address and initial value " << initialValue << std::endl;
#endif
    new (static_cast<void*>(address)) T(initialValue);
  }

  void construct(pointer address) {
#if ENABLE_ALLOCATOR_LOG
    std::cout << "Called MemstackAllocator::construct with 0x" << address
              << " address" << std::endl;
#endif
    new (static_cast<void*>(address)) T();
  }

  void destroy(pointer address) {
#if ENABLE_ALLOCATOR_LOG
    std::cout << "Called MemstackAllocator::destroy with 0x" << address
              << " address" << std::endl;
#endif
    address->~T();
  }

  // ======= END: public misc methods   =======================================
};

}  // namespace game_engine

#endif  // GAME_ENGINE_MEM_STACK_ALLOCATOR_H