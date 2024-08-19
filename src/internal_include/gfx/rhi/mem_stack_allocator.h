#ifndef GAME_ENGINE_MEM_STACK_ALLOCATOR_H
#define GAME_ENGINE_MEM_STACK_ALLOCATOR_H

#define ENABLE_ALLOCATOR_LOG 0
#define DEFAULT_ALIGNMENT    16

#include "gfx/rhi/lock.h"
#include "utils/memory/align.h"

#include <cassert>

namespace game_engine {

struct MemoryChunk {
  MemoryChunk() = default;

  MemoryChunk(uint8_t* address, uint64_t offset, uint64_t dataSize)
      : m_address_(address)
      , m_offset_(offset)
      , m_dataSize_(dataSize) {}

  void* Alloc(size_t numOfBytes) {
    const uint64_t allocOffset = Align<uint64_t>(m_offset_, DEFAULT_ALIGNMENT);
    if (allocOffset + numOfBytes <= m_dataSize_) {
      m_offset_ = allocOffset + numOfBytes;
      return m_address_ + allocOffset;
    }
    return nullptr;
  }

  uint8_t* m_address_  = nullptr;
  uint64_t m_offset_   = 0;
  uint64_t m_dataSize_ = 0;

  MemoryChunk* m_next_ = nullptr;
};

class PageAllocator {
  public:
  static constexpr uint64_t s_kPageSize = 1024 * 4;
  static constexpr uint64_t s_kMaxMemoryChunkSize
      = s_kPageSize - sizeof(MemoryChunk);

  static PageAllocator* Get() {
    static PageAllocator* s_pageAllocator = nullptr;
    if (!s_pageAllocator) {
      s_pageAllocator = new PageAllocator();
    }

    return s_pageAllocator;
  }

  MemoryChunk* Allocate() {
    {
      ScopedLock s(&m_lock_);
      if (m_freeChunk_) {
        MemoryChunk* Chunk = m_freeChunk_;
        m_freeChunk_       = m_freeChunk_->m_next_;
        return Chunk;
      }
    }

    uint8_t*     NewLowMemory   = static_cast<uint8_t*>(malloc(s_kPageSize));
    MemoryChunk* NewMemoryChunk = new (NewLowMemory) MemoryChunk(
        NewLowMemory, sizeof(MemoryChunk), PageAllocator::s_kPageSize);
    return NewMemoryChunk;
  }

  void Free(MemoryChunk* chunk) {
    ScopedLock s(&m_lock_);

    chunk->m_offset_ = sizeof(MemoryChunk);
    chunk->m_next_   = m_freeChunk_;
    m_freeChunk_     = chunk;
  }

  MemoryChunk* AllocateBigSize(uint64_t numOfBytes) {
    const uint64_t NeedToAllocateBytes = numOfBytes + sizeof(MemoryChunk);

    if (m_freeChunkBigSize_) {
      MemoryChunk* ChunkPrev = nullptr;
      MemoryChunk* CurChunk  = m_freeChunkBigSize_;
      while (CurChunk) {
        if (CurChunk->m_dataSize_ >= NeedToAllocateBytes) {
          if (ChunkPrev) {
            ChunkPrev->m_next_ = CurChunk->m_next_;
          }

          return CurChunk;
        }

        ChunkPrev = CurChunk;
        CurChunk  = CurChunk->m_next_;
      }
    }

    uint8_t* NewLowMemory = static_cast<uint8_t*>(malloc(NeedToAllocateBytes));
    MemoryChunk* NewChunk = new (NewLowMemory)
        MemoryChunk(NewLowMemory, sizeof(MemoryChunk), NeedToAllocateBytes);
    return NewChunk;
  }

  void FreeBigSize(MemoryChunk* chunk) {
    chunk->m_offset_ = sizeof(MemoryChunk);

    if (m_freeChunkBigSize_) {
      MemoryChunk* ChunkPrev = nullptr;
      MemoryChunk* CurChunk  = m_freeChunkBigSize_;
      while (CurChunk) {
        if (CurChunk->m_dataSize_ >= chunk->m_dataSize_) {
          if (ChunkPrev) {
            ChunkPrev->m_next_ = chunk;
          }
          chunk->m_next_ = CurChunk;
          return;
        }

        ChunkPrev = CurChunk;
        CurChunk  = CurChunk->m_next_;
      }

      if (ChunkPrev) {
        ChunkPrev->m_next_ = chunk;
        return;
      }
    }

    m_freeChunkBigSize_ = chunk;
  }

  void Flush() {
    ScopedLock s(&m_lock_);
    while (m_freeChunk_) {
      MemoryChunk* Next = m_freeChunk_->m_next_;
      delete[] m_freeChunk_;
      m_freeChunk_ = Next;
    }
  }

  MutexLock    m_lock_;
  MemoryChunk* m_freeChunk_        = nullptr;
  MemoryChunk* m_freeChunkBigSize_ = nullptr;

  private:
  PageAllocator()                                   = default;
  PageAllocator(const PageAllocator&)               = delete;
  PageAllocator(PageAllocator&&)                    = delete;
  PageAllocator& operator=(const PageAllocator& In) = delete;
};

class MemStack {
  public:
  static MemStack* Get() {
    static MemStack* s_memStack = nullptr;
    if (!s_memStack) {
      s_memStack = new MemStack();
    }

    return s_memStack;
  }

  template <typename T>
  T* Alloc() {
    return (T*)Alloc(sizeof(T));
  }

  void* Alloc(uint64_t numOfBytes) {
    if (numOfBytes >= PageAllocator::s_kMaxMemoryChunkSize) {
      MemoryChunk* NewChunk
          = PageAllocator::Get()->AllocateBigSize(numOfBytes);
      assert(NewChunk);

      NewChunk->m_next_ = m_bigSizeChunk_;
      m_bigSizeChunk_   = NewChunk;

      void* AllocatedMemory = m_bigSizeChunk_->Alloc(numOfBytes);
      assert(AllocatedMemory);
      return AllocatedMemory;
    }

    if (m_topMemoryChunk_) {
      void* AllocatedMemory = m_topMemoryChunk_->Alloc(numOfBytes);
      if (AllocatedMemory) {
        return AllocatedMemory;
      }
    }

    MemoryChunk* NewChunk = PageAllocator::Get()->Allocate();
    assert(NewChunk);

    NewChunk->m_next_ = m_topMemoryChunk_;
    m_topMemoryChunk_ = NewChunk;

    void* AllocatedMemory = m_topMemoryChunk_->Alloc(numOfBytes);
    assert(AllocatedMemory);
    return AllocatedMemory;
  }

  void Flush() {
    while (m_topMemoryChunk_) {
      MemoryChunk* Next = m_topMemoryChunk_->m_next_;
      PageAllocator::Get()->Free(m_topMemoryChunk_);
      m_topMemoryChunk_ = Next;
    }

    while (m_bigSizeChunk_) {
      MemoryChunk* Next = m_bigSizeChunk_->m_next_;
      PageAllocator::Get()->FreeBigSize(m_bigSizeChunk_);
      m_bigSizeChunk_ = Next;
    }
  }

  private:
  MemoryChunk* m_topMemoryChunk_ = nullptr;
  MemoryChunk* m_bigSizeChunk_   = nullptr;
};

template <typename T>
class MemStackAllocator {
  public:
  typedef size_t    size_type;
  typedef ptrdiff_t difference_type;
  typedef T*        pointer;
  typedef const T*  const_pointer;
  typedef T&        reference;
  typedef const T&  const_reference;
  typedef T         value_type;

  // TODO: refactor numOfElements
  pointer allocate(size_t numOfElement) {
#if ENABLE_ALLOCATOR_LOG
    pointer allocatedAddress = static_cast<pointer>(
        MemStack::Get()->Alloc(numOfElement * sizeof(T)));
    std::cout << "Called MemstackAllocator::allocate with 0x"
              << allocatedAddress << " address and " << numOfElement
              << " elements" << std::endl;
    return allocatedAddress;
#else
    return static_cast<pointer>(
        MemStack::Get()->Alloc(numOfElement * sizeof(T)));
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
};

}  // namespace game_engine

#endif  // GAME_ENGINE_MEM_STACK_ALLOCATOR_H