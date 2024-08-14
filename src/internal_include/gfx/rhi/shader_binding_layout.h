#ifndef GAME_ENGINE_SHADER_BINDING_LAYOUT_H
#define GAME_ENGINE_SHADER_BINDING_LAYOUT_H

#include "gfx/rhi/buffer.h"
#include "gfx/rhi/i_uniform_buffer_block.h"
// #include "gfx/rhi/pipeline_state_info.h" // circular dependency
#include "gfx/rhi/resource_container.h"
#include "gfx/rhi/rhi_type.h"
#include "gfx/rhi/texture.h"

#include <cassert>
#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

namespace game_engine {

struct SamplerStateInfo;

// Single resources
struct ShaderBindingResource
/*: public std::enable_shared_from_this<ShaderBindingResource>*/ {
  virtual ~ShaderBindingResource() {}

  virtual const void* GetResource() const { return nullptr; }

  virtual int32_t NumOfResource() const { return 1; }

  virtual bool IsBindless() const { return false; }
};

struct UniformBufferResource : public ShaderBindingResource {
  UniformBufferResource() = default;

  UniformBufferResource(const IUniformBufferBlock* InUniformBuffer)
      : UniformBuffer(InUniformBuffer) {}

  virtual ~UniformBufferResource() {}

  virtual const void* GetResource() const override { return UniformBuffer; }

  const IUniformBufferBlock* UniformBuffer = nullptr;
};

struct BufferResource : public ShaderBindingResource {
  BufferResource() = default;

  BufferResource(const Buffer* InBuffer)
      : m_buffer(InBuffer) {}

  virtual ~BufferResource() {}

  virtual const void* GetResource() const override { return m_buffer; }

  const Buffer* m_buffer = nullptr;
};

struct SamplerResource : public ShaderBindingResource {
  SamplerResource() = default;

  SamplerResource(const SamplerStateInfo* InSamplerState)
      : SamplerState(InSamplerState) {}

  virtual ~SamplerResource() {}

  virtual const void* GetResource() const override { return SamplerState; }

  const SamplerStateInfo* SamplerState = nullptr;
};

struct TextureResource : public SamplerResource {
  TextureResource() = default;

  TextureResource(const Texture*          InTexture,
                  const SamplerStateInfo* InSamplerState,
                  int32_t                 InMipLevel = 0)
      : SamplerResource(InSamplerState)
      , m_texture(InTexture)
      , MipLevel(InMipLevel) {}

  virtual ~TextureResource() {}

  virtual const void* GetResource() const override { return m_texture; }

  const Texture* m_texture = nullptr;
  const int32_t  MipLevel  = 0;
};

struct TextureArrayResource : public ShaderBindingResource {
  TextureArrayResource() = default;

  TextureArrayResource(const Texture** InTextureArray,
                       const int32_t   InNumOfTexure)
      : TextureArray(InTextureArray)
      , NumOfTexure(InNumOfTexure) {}

  virtual ~TextureArrayResource() {}

  virtual const void* GetResource() const override { return TextureArray; }

  virtual int32_t NumOfResource() const override { return NumOfTexure; }

  const Texture** TextureArray = nullptr;
  const int32_t   NumOfTexure  = 1;
};

//////////////////////////////////////////////////////////////////////////

// Bindless resources, It contain multiple resources at once.
struct UniformBufferResourceBindless : public ShaderBindingResource {
  UniformBufferResourceBindless() = default;

  UniformBufferResourceBindless(
      const std::vector<const IUniformBufferBlock*>& InUniformBuffers)
      : UniformBuffers(InUniformBuffers) {}

  virtual ~UniformBufferResourceBindless() {}

  virtual const void* GetResource(int32_t InIndex) const {
    return UniformBuffers[InIndex];
  }

  virtual int32_t GetNumOfResources() const {
    return (int32_t)UniformBuffers.size();
  }

  virtual bool IsBindless() const { return true; }

  std::vector<const IUniformBufferBlock*> UniformBuffers;
};

struct BufferResourceBindless : public ShaderBindingResource {
  BufferResourceBindless() = default;

  BufferResourceBindless(const std::vector<const Buffer*>& InBuffers)
      : m_buffers(InBuffers) {}

  virtual ~BufferResourceBindless() {}

  virtual const void* GetResource(int32_t InIndex) const {
    return m_buffers[InIndex];
  }

  virtual bool IsBindless() const { return true; }

  std::vector<const Buffer*> m_buffers;
};

struct SamplerResourceBindless : public ShaderBindingResource {
  SamplerResourceBindless() = default;

  SamplerResourceBindless(
      const std::vector<const SamplerStateInfo*>& InSamplerStates)
      : SamplerStates(InSamplerStates) {}

  virtual ~SamplerResourceBindless() {}

  virtual const void* GetResource(int32_t InIndex) const {
    return SamplerStates[InIndex];
  }

  virtual bool IsBindless() const { return true; }

  std::vector<const SamplerStateInfo*> SamplerStates;
};

struct TextureResourceBindless : public ShaderBindingResource {
  struct TextureBindData {
    TextureBindData() = default;

    TextureBindData(Texture*          InTexture,
                    SamplerStateInfo* InSamplerState,
                    int32_t           InMipLevel = 0)
        : m_texture(InTexture)
        , SamplerState(InSamplerState)
        , MipLevel(InMipLevel) {}

    Texture*          m_texture    = nullptr;
    SamplerStateInfo* SamplerState = nullptr;
    int32_t           MipLevel     = 0;
  };

  TextureResourceBindless() = default;

  TextureResourceBindless(const std::vector<TextureBindData>& InTextureBindData)
      : TextureBindDatas(InTextureBindData) {}

  virtual ~TextureResourceBindless() {}

  virtual const void* GetResource(int32_t InIndex) const {
    return &TextureBindDatas[0];
  }

  virtual bool IsBindless() const { return true; }

  std::vector<TextureBindData> TextureBindDatas;
};

struct TextureArrayResourceBindless : public ShaderBindingResource {
  struct TextureArrayBindData {
    Texture** TextureArray  = nullptr;
    int32_t   InNumOfTexure = 0;
  };

  TextureArrayResourceBindless() = default;

  TextureArrayResourceBindless(
      const std::vector<TextureArrayBindData>& InTextureArrayBindDatas)
      : TextureArrayBindDatas(InTextureArrayBindDatas) {}

  virtual ~TextureArrayResourceBindless() {}

  virtual const void* GetResource(int32_t InIndex) const {
    return &TextureArrayBindDatas[InIndex];
  }

  virtual int32_t NumOfResource(int32_t InIndex) const {
    return TextureArrayBindDatas[InIndex].InNumOfTexure;
  }

  virtual bool IsBindless() const { return true; }

  std::vector<TextureArrayBindData> TextureArrayBindDatas;
};

//////////////////////////////////////////////////////////////////////////

struct ShaderBindingResourceInlineAllocator {
  template <typename T, typename... T1>
  T* Alloc(T1... args) {
    assert((Offset + sizeof(T)) < sizeof(Data));

    T* AllocatedAddress  = new (&Data[0] + Offset) T(args...);
    Offset              += sizeof(T);
    return AllocatedAddress;
  }

  void Reset() { Offset = 0; }

  uint8_t Data[1024];
  int32_t Offset = 0;
};

struct ShaderBinding {
  static constexpr int32_t APPEND_LAST = -1;  // BindingPoint is appending last

  ShaderBinding() = default;

  ShaderBinding(const int32_t                InBindingPoint,
                const int32_t                InNumOfDescriptors,
                const EShaderBindingType     InBindingType,
                const bool                   InIsBindless,
                const EShaderAccessStageFlag InAccessStageFlags,
                const ShaderBindingResource* InResource = nullptr,
                bool                         InIsInline = false)
      : BindingPoint(InBindingPoint)
      , NumOfDescriptors(InNumOfDescriptors)
      , BindingType(InBindingType)
      , IsBindless(InIsBindless)
      , AccessStageFlags(InAccessStageFlags)
      , Resource(InResource)
      , IsInline(InIsInline) {
    // SubpassInputAttachment must have the stageflag 0.
    assert(EShaderBindingType::SUBPASS_INPUT_ATTACHMENT != InBindingType
           || InAccessStageFlags == EShaderAccessStageFlag::FRAGMENT);

    if (InResource) {
      assert(InResource->IsBindless() == InIsBindless);
    }

    GetHash();
  }

  size_t GetHash() const {
    if (Hash) {
      return Hash;
    }

    Hash = GETHASH_FROM_INSTANT_STRUCT(IsInline,
                                       BindingPoint,
                                       NumOfDescriptors,
                                       BindingType,
                                       AccessStageFlags,
                                       IsBindless);
    return Hash;
  }

  void CloneWithoutResource(ShaderBinding& OutReslut) const {
    OutReslut.IsInline         = IsInline;
    OutReslut.BindingPoint     = BindingPoint;
    OutReslut.NumOfDescriptors = NumOfDescriptors;
    OutReslut.BindingType      = BindingType;
    OutReslut.AccessStageFlags = AccessStageFlags;
    OutReslut.IsBindless       = IsBindless;
    OutReslut.Hash             = Hash;
  }

  mutable size_t Hash = 0;

  bool                   IsInline         = false;
  bool                   IsBindless       = false;
  int32_t                BindingPoint     = 0;
  int32_t                NumOfDescriptors = 1;
  EShaderBindingType     BindingType      = EShaderBindingType::UNIFORMBUFFER;
  EShaderAccessStageFlag AccessStageFlags
      = EShaderAccessStageFlag::ALL_GRAPHICS;

  const ShaderBindingResource* Resource = nullptr;
};

struct ShaderBindingArray {
  static constexpr int32_t NumOfInlineData = 10;

  template <typename... T>
  void Add(T... args) {
    static_assert(std::is_trivially_copyable<ShaderBinding>::value,
                  "ShaderBinding should be trivially copyable");

    assert(NumOfInlineData > NumOfData);
    new (&Data[NumOfData]) ShaderBinding(args...);
    ++NumOfData;
  }

  void Add(const ShaderBinding& args) {
    static_assert(std::is_trivially_copyable<ShaderBinding>::value,
                  "ShaderBinding should be trivially copyable");

    assert(NumOfInlineData > NumOfData);
    Data[NumOfData] = args;
    ++NumOfData;
  }

  size_t GetHash() const {
    size_t         Hash    = 0;
    ShaderBinding* Address = (ShaderBinding*)&Data[0];
    for (int32_t i = 0; i < NumOfData; ++i) {
      Hash ^= ((Address + i)->GetHash() << i);
    }
    return Hash;
  }

  ShaderBindingArray& operator=(const ShaderBindingArray& In) {
    memcpy(&Data[0], &In.Data[0], sizeof(ShaderBinding) * In.NumOfData);
    NumOfData = In.NumOfData;
    return *this;
  }

  const ShaderBinding* operator[](int32_t InIndex) const {
    assert(InIndex < NumOfData);
    return (ShaderBinding*)(&Data[InIndex]);
  }

  void CloneWithoutResource(ShaderBindingArray& OutResult) const {
    memcpy(&OutResult.Data[0], &Data[0], sizeof(ShaderBinding) * NumOfData);

    for (int32_t i = 0; i < NumOfData; ++i) {
      ShaderBinding* SrcAddress = (ShaderBinding*)&Data[i];
      ShaderBinding* DstAddress = (ShaderBinding*)&OutResult.Data[i];
      SrcAddress->CloneWithoutResource(*DstAddress);
    }
    OutResult.NumOfData = NumOfData;
  }

  ShaderBinding Data[NumOfInlineData];
  int32_t       NumOfData = 0;
};

template <typename T>
struct TShaderBinding : public ShaderBinding {
  TShaderBinding(const int32_t                InBindingPoint,
                 const int32_t                InNumOfDescriptors,
                 const EShaderBindingType     InBindingType,
                 const EShaderAccessStageFlag InAccessStageFlags,
                 const T&                     InData)
      : ShaderBinding(
          InBindingPoint, InNumOfDescriptors, InBindingType, InAccessStageFlags)
      , Data(InData) {}

  T Data = T();
};

// struct ShaderBindingArray {
//   static constexpr int32_t NumOfInlineBindings = 10;
//
//   struct Allocator {
//     uint8* InlineStrage[NumOfInlineBindings] = {};
//
//     uint8* GetAddress() const { return InlineStrage[0]; }
//
//     void SetHeapMemeory(uint8* InHeap) { InlineStrage[0] = InHeap; }
//   };
//
//   Allocator Data{};
//   uint64_t    AllocateOffset = 0;
//
//   template <typename T>
//   void Add(const TShaderBinding<T>& InShaderBinding) {
//     const bool NotEnoughInlineStorage
//         = (sizeof(Data) < AllocateOffset + sizeof(InShaderBinding));
//     if (NotEnoughInlineStorage) {
//       // Allocate additional memory
//       assert(0);
//     }
//
//     memcpy(Data.GetAddress() + AllocateOffset,
//            InShaderBinding,
//            sizeof(InShaderBinding));
//   }
// };

enum class ShaderBindingInstanceType : uint8_t {
  SingleFrame = 0,
  MultiFrame,
  Max
};

struct ShaderBindingInstance
    : public std::enable_shared_from_this<ShaderBindingInstance> {
  virtual ~ShaderBindingInstance() {}

  const struct ShaderBindingLayout* ShaderBindingsLayouts = nullptr;

  virtual void Initialize(const ShaderBindingArray& InShaderBindingArray) {}

  virtual void UpdateShaderBindings(
      const ShaderBindingArray& InShaderBindingArray) {}

  virtual void* GetHandle() const { return nullptr; }

  virtual const std::vector<uint32_t>* GetDynamicOffsets() const {
    return nullptr;
  }

  virtual void Free() {}

  virtual ShaderBindingInstanceType GetType() const { return Type; }

  virtual void SetType(const ShaderBindingInstanceType InType) {
    Type = InType;
  }

  private:
  ShaderBindingInstanceType Type = ShaderBindingInstanceType::SingleFrame;
};

// todo : MemStack for ShaderBindingInstanceArray to allocate fast memory
using ShaderBindingInstanceArray
    = ResourceContainer<const ShaderBindingInstance*>;
using ShaderBindingInstancePtrArray
    = std::vector<std::shared_ptr<ShaderBindingInstance>>;

struct ShaderBindingLayout {
  virtual ~ShaderBindingLayout() {}

  virtual bool Initialize(const ShaderBindingArray& InShaderBindingArray) {
    return false;
  }

  virtual std::shared_ptr<ShaderBindingInstance> CreateShaderBindingInstance(
      const ShaderBindingArray&       InShaderBindingArray,
      const ShaderBindingInstanceType InType) const {
    return nullptr;
  }

  virtual size_t GetHash() const {
    if (Hash) {
      return Hash;
    }

    Hash = m_shaderBindingArray_.GetHash();
    return Hash;
  }

  virtual const ShaderBindingArray& GetShaderBindingsLayout() const {
    return m_shaderBindingArray_;
  }

  virtual void* GetHandle() const { return nullptr; }

  mutable size_t Hash = 0;

  protected:
  ShaderBindingArray m_shaderBindingArray_;  // Resource information is empty
};

using ShaderBindingLayoutArray = ResourceContainer<const ShaderBindingLayout*>;

// TODO: rename namespace + consider anonymous namespace. Also write doxygen
// comments explaining why to use this
namespace test {
uint32_t GetCurrentFrameNumber();
}  // namespace test

// To pending deallocation for MultiFrame GPU Data, Because Avoding deallocation
// inflighting GPU Data (ex. ShaderBindingInstance, IUniformBufferBlock)
template <typename T>
struct DeallocatorMultiFrameResource {
  static constexpr int32_t NumOfFramesToWaitBeforeReleasing = 3;

  struct PendingFreeData {
    PendingFreeData() = default;

    PendingFreeData(int32_t InFrameIndex, std::shared_ptr<T> InDataPtr)
        : FrameIndex(InFrameIndex)
        , DataPtr(InDataPtr) {}

    int32_t            FrameIndex = 0;
    std::shared_ptr<T> DataPtr    = nullptr;
  };

  std::vector<PendingFreeData> PendingFree;
  int32_t                      CanReleasePendingFreeDataFrameNumber = 0;
  std::function<void(std::shared_ptr<T>)> FreeDelegate;

  void Free(std::shared_ptr<T> InDataPtr) {
    const int32_t CurrentFrameNumber = test::GetCurrentFrameNumber();
    const int32_t OldestFrameToKeep
        = CurrentFrameNumber - NumOfFramesToWaitBeforeReleasing;

    // ProcessPendingDescriptorPoolFree
    {
      // Check it is too early
      if (CurrentFrameNumber >= CanReleasePendingFreeDataFrameNumber) {
        // Release pending memory
        int32_t i = 0;
        for (; i < PendingFree.size(); ++i) {
          PendingFreeData& pendingFreeData = PendingFree[i];
          if (pendingFreeData.FrameIndex < OldestFrameToKeep) {
            // Return to pending descriptor set
            if (FreeDelegate) {
              FreeDelegate(pendingFreeData.DataPtr);
            }
          } else {
            CanReleasePendingFreeDataFrameNumber
                = pendingFreeData.FrameIndex + NumOfFramesToWaitBeforeReleasing
                + 1;
            break;
          }
        }
        if (i > 0) {
          const size_t RemainingSize = (PendingFree.size() - i);
          if (RemainingSize > 0) {
            for (int32_t k = 0; k < RemainingSize; ++k) {
              PendingFree[k] = PendingFree[i + k];
            }
          }
          PendingFree.resize(RemainingSize);
        }
      }
    }

    PendingFree.emplace_back(PendingFreeData(CurrentFrameNumber, InDataPtr));
  }
};

using DeallocatorMultiFrameShaderBindingInstance
    = DeallocatorMultiFrameResource<ShaderBindingInstance>;
// using DeallocatorMultiFrameUniformBufferBlock
//     = DeallocatorMultiFrameResource<IUniformBufferBlock>;

}  // namespace game_engine

#endif  // GAME_ENGINE_SHADER_BINDING_LAYOUT_H