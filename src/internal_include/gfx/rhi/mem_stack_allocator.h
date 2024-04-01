#ifndef GAME_ENGINE_MEM_STACK_ALLOCATOR_H
#define GAME_ENGINE_MEM_STACK_ALLOCATOR_H

#define ENABLE_ALLOCATOR_LOG 0
#define DEFAULT_ALIGNMENT    16

#include "gfx/rhi/lock.h"

#include <cassert>

namespace game_engine {

struct MemoryChunk {
  MemoryChunk() = default;

  MemoryChunk(uint8_t* InAddress, uint64_t InOffset, uint64_t InDataSize)
      : Address(InAddress)
      , Offset(InOffset)
      , DataSize(InDataSize) {}

  uint8_t* Address  = nullptr;
  uint64_t Offset   = 0;
  uint64_t DataSize = 0;

  MemoryChunk* Next = nullptr;

  void* Alloc(size_t InNumOfBytes) {
    const uint64_t allocOffset = Align<uint64_t>(Offset, DEFAULT_ALIGNMENT);
    if (allocOffset + InNumOfBytes <= DataSize) {
      Offset = allocOffset + InNumOfBytes;
      return Address + allocOffset;
    }
    return nullptr;
  }
};

class PageAllocator {
  public:
  static constexpr uint64_t PageSize           = 1024 * 4;
  static constexpr uint64_t MaxMemoryChunkSize = PageSize - sizeof(MemoryChunk);

  static PageAllocator* Get() {
    static PageAllocator* s_pageAllocator = nullptr;
    if (!s_pageAllocator) {
      s_pageAllocator = new PageAllocator();
    }

    return s_pageAllocator;
  }

  MemoryChunk* Allocate() {
    {
      ScopedLock s(&Lock);
      if (FreeChunk) {
        MemoryChunk* Chunk = FreeChunk;
        FreeChunk           = FreeChunk->Next;
        return Chunk;
      }
    }

    uint8_t*        NewLowMemory   = static_cast<uint8_t*>(malloc(PageSize));
    MemoryChunk* NewMemoryChunk = new (NewLowMemory) MemoryChunk(
        NewLowMemory, sizeof(MemoryChunk), PageAllocator::PageSize);
    return NewMemoryChunk;
  }

  void Free(MemoryChunk* InChunk) {
    ScopedLock s(&Lock);

    InChunk->Offset = sizeof(MemoryChunk);
    InChunk->Next   = FreeChunk;
    FreeChunk       = InChunk;
  }

  MemoryChunk* AllocateBigSize(uint64_t InNumOfBytes) {
    const uint64_t NeedToAllocateBytes = InNumOfBytes + sizeof(MemoryChunk);

    if (FreeChunkBigSize) {
      MemoryChunk* ChunkPrev = nullptr;
      MemoryChunk* CurChunk  = FreeChunkBigSize;
      while (CurChunk) {
        if (CurChunk->DataSize >= NeedToAllocateBytes) {
          if (ChunkPrev) {
            ChunkPrev->Next = CurChunk->Next;
          }

          return CurChunk;
        }

        ChunkPrev = CurChunk;
        CurChunk  = CurChunk->Next;
      }
    }

    uint8_t* NewLowMemory    = static_cast<uint8_t*>(malloc(NeedToAllocateBytes));
    MemoryChunk* NewChunk = new (NewLowMemory)
        MemoryChunk(NewLowMemory, sizeof(MemoryChunk), NeedToAllocateBytes);
    return NewChunk;
  }

  void FreeBigSize(MemoryChunk* InChunk) {
    InChunk->Offset = sizeof(MemoryChunk);

    if (FreeChunkBigSize) {
      MemoryChunk* ChunkPrev = nullptr;
      MemoryChunk* CurChunk  = FreeChunkBigSize;
      while (CurChunk) {
        if (CurChunk->DataSize >= InChunk->DataSize) {
          if (ChunkPrev) {
            ChunkPrev->Next = InChunk;
          }
          InChunk->Next = CurChunk;
          return;
        }

        ChunkPrev = CurChunk;
        CurChunk  = CurChunk->Next;
      }

      if (ChunkPrev) {
        ChunkPrev->Next = InChunk;
        return;
      }
    }

    FreeChunkBigSize = InChunk;
  }

  void Flush() {
    ScopedLock s(&Lock);
    while (FreeChunk) {
      MemoryChunk* Next = FreeChunk->Next;
      delete[] FreeChunk;
      FreeChunk = Next;
    }
  }

  MutexLock    Lock;
  MemoryChunk* FreeChunk        = nullptr;
  MemoryChunk* FreeChunkBigSize = nullptr;

  private:
  PageAllocator()                                    = default;
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

  void* Alloc(uint64_t InNumOfBytes) {
    if (InNumOfBytes >= PageAllocator::MaxMemoryChunkSize) {
      MemoryChunk* NewChunk
          = PageAllocator::Get()->AllocateBigSize(InNumOfBytes);
      assert(NewChunk);

      NewChunk->Next = BigSizeChunk;
      BigSizeChunk   = NewChunk;

      void* AllocatedMemory = BigSizeChunk->Alloc(InNumOfBytes);
      assert(AllocatedMemory);
      return AllocatedMemory;
    }

    if (TopMemoryChunk) {
      void* AllocatedMemory = TopMemoryChunk->Alloc(InNumOfBytes);
      if (AllocatedMemory) {
        return AllocatedMemory;
      }
    }

    MemoryChunk* NewChunk = PageAllocator::Get()->Allocate();
    assert(NewChunk);

    NewChunk->Next = TopMemoryChunk;
    TopMemoryChunk = NewChunk;

    void* AllocatedMemory = TopMemoryChunk->Alloc(InNumOfBytes);
    assert(AllocatedMemory);
    return AllocatedMemory;
  }

  void Flush() {
    while (TopMemoryChunk) {
      MemoryChunk* Next = TopMemoryChunk->Next;
      PageAllocator::Get()->Free(TopMemoryChunk);
      TopMemoryChunk = Next;
    }

    while (BigSizeChunk) {
      MemoryChunk* Next = BigSizeChunk->Next;
      PageAllocator::Get()->FreeBigSize(BigSizeChunk);
      BigSizeChunk = Next;
    }
  }

  private:
  MemoryChunk* TopMemoryChunk = nullptr;
  MemoryChunk* BigSizeChunk   = nullptr;
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

  pointer allocate(size_t InNumOfElement) {
#if ENABLE_ALLOCATOR_LOG
    pointer allocatedAddress = static_cast<pointer>(
        MemStack::Get()->Alloc(InNumOfElement * sizeof(T)));
    std::cout << "Called jMemstackAllocator::allocate with 0x"
              << allocatedAddress << " address and " << InNumOfElement
              << " elements" << std::endl;
    return allocatedAddress;
#else
    return static_cast<pointer>(
        MemStack::Get()->Alloc(InNumOfElement * sizeof(T)));
#endif
  }

  void deallocate(pointer InAddress, size_t InNumOfElement) {
#if ENABLE_ALLOCATOR_LOG
    std::cout << "Called jMemstackAllocator::deallocate with 0x" << InAddress
              << " address and " << InNumOfElement << " elements" << std::endl;
#endif
    // free(InAddress);
  }

  void construct(pointer InAddress, const const_reference InInitialValue) {
#if ENABLE_ALLOCATOR_LOG
    std::cout << "Called jMemstackAllocator::construct with 0x" << InAddress
              << " address and initial value " << InInitialValue << std::endl;
#endif
    new (static_cast<void*>(InAddress)) T(InInitialValue);
  }

  void construct(pointer InAddress) {
#if ENABLE_ALLOCATOR_LOG
    std::cout << "Called jMemstackAllocator::construct with 0x" << InAddress
              << " address" << std::endl;
#endif
    new (static_cast<void*>(InAddress)) T();
  }

  void destroy(pointer InAddress) {
#if ENABLE_ALLOCATOR_LOG
    std::cout << "Called jMemstackAllocator::destroy with 0x" << InAddress
              << " address" << std::endl;
#endif
    InAddress->~T();
  }
};

}  // namespace game_engine

#endif                    // GAME_ENGINE_MEM_STACK_ALLOCATOR_H