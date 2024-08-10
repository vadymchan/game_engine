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

struct jSamplerStateInfo;

// Single resources
struct jShaderBindingResource
/*: public std::enable_shared_from_this<jShaderBindingResource>*/ {
  virtual ~jShaderBindingResource() {}

  virtual const void* GetResource() const { return nullptr; }

  virtual int32_t NumOfResource() const { return 1; }

  virtual bool IsBindless() const { return false; }
};

struct jUniformBufferResource : public jShaderBindingResource {
  jUniformBufferResource() = default;

  jUniformBufferResource(const IUniformBufferBlock* InUniformBuffer)
      : UniformBuffer(InUniformBuffer) {}

  virtual ~jUniformBufferResource() {}

  virtual const void* GetResource() const override { return UniformBuffer; }

  const IUniformBufferBlock* UniformBuffer = nullptr;
};

struct jBufferResource : public jShaderBindingResource {
  jBufferResource() = default;

  jBufferResource(const jBuffer* InBuffer)
      : Buffer(InBuffer) {}

  virtual ~jBufferResource() {}

  virtual const void* GetResource() const override { return Buffer; }

  const jBuffer* Buffer = nullptr;
};

struct jSamplerResource : public jShaderBindingResource {
  jSamplerResource() = default;

  jSamplerResource(const jSamplerStateInfo* InSamplerState)
      : SamplerState(InSamplerState) {}

  virtual ~jSamplerResource() {}

  virtual const void* GetResource() const override { return SamplerState; }

  const jSamplerStateInfo* SamplerState = nullptr;
};

struct jTextureResource : public jSamplerResource {
  jTextureResource() = default;

  jTextureResource(const jTexture*          InTexture,
                   const jSamplerStateInfo* InSamplerState,
                   int32_t                  InMipLevel = 0)
      : jSamplerResource(InSamplerState)
      , Texture(InTexture)
      , MipLevel(InMipLevel) {}

  virtual ~jTextureResource() {}

  virtual const void* GetResource() const override { return Texture; }

  const jTexture* Texture  = nullptr;
  const int32_t   MipLevel = 0;
};

struct jTextureArrayResource : public jShaderBindingResource {
  jTextureArrayResource() = default;

  jTextureArrayResource(const jTexture** InTextureArray,
                        const int32_t    InNumOfTexure)
      : TextureArray(InTextureArray)
      , NumOfTexure(InNumOfTexure) {}

  virtual ~jTextureArrayResource() {}

  virtual const void* GetResource() const override { return TextureArray; }

  virtual int32_t NumOfResource() const override { return NumOfTexure; }

  const jTexture** TextureArray = nullptr;
  const int32_t    NumOfTexure  = 1;
};

//////////////////////////////////////////////////////////////////////////

// Bindless resources, It contain multiple resources at once.
struct jUniformBufferResourceBindless : public jShaderBindingResource {
  jUniformBufferResourceBindless() = default;

  jUniformBufferResourceBindless(
      const std::vector<const IUniformBufferBlock*>& InUniformBuffers)
      : UniformBuffers(InUniformBuffers) {}

  virtual ~jUniformBufferResourceBindless() {}

  virtual const void* GetResource(int32_t InIndex) const {
    return UniformBuffers[InIndex];
  }

  virtual int32_t GetNumOfResources() const {
    return (int32_t)UniformBuffers.size();
  }

  virtual bool IsBindless() const { return true; }

  std::vector<const IUniformBufferBlock*> UniformBuffers;
};

struct jBufferResourceBindless : public jShaderBindingResource {
  jBufferResourceBindless() = default;

  jBufferResourceBindless(const std::vector<const jBuffer*>& InBuffers)
      : Buffers(InBuffers) {}

  virtual ~jBufferResourceBindless() {}

  virtual const void* GetResource(int32_t InIndex) const {
    return Buffers[InIndex];
  }

  virtual bool IsBindless() const { return true; }

  std::vector<const jBuffer*> Buffers;
};

struct jSamplerResourceBindless : public jShaderBindingResource {
  jSamplerResourceBindless() = default;

  jSamplerResourceBindless(
      const std::vector<const jSamplerStateInfo*>& InSamplerStates)
      : SamplerStates(InSamplerStates) {}

  virtual ~jSamplerResourceBindless() {}

  virtual const void* GetResource(int32_t InIndex) const {
    return SamplerStates[InIndex];
  }

  virtual bool IsBindless() const { return true; }

  std::vector<const jSamplerStateInfo*> SamplerStates;
};

struct jTextureResourceBindless : public jShaderBindingResource {
  struct jTextureBindData {
    jTextureBindData() = default;

    jTextureBindData(jTexture*          InTexture,
                     jSamplerStateInfo* InSamplerState,
                     int32_t            InMipLevel = 0)
        : Texture(InTexture)
        , SamplerState(InSamplerState)
        , MipLevel(InMipLevel) {}

    jTexture*          Texture      = nullptr;
    jSamplerStateInfo* SamplerState = nullptr;
    int32_t            MipLevel     = 0;
  };

  jTextureResourceBindless() = default;

  jTextureResourceBindless(
      const std::vector<jTextureBindData>& InTextureBindData)
      : TextureBindDatas(InTextureBindData) {}

  virtual ~jTextureResourceBindless() {}

  virtual const void* GetResource(int32_t InIndex) const {
    return &TextureBindDatas[0];
  }

  virtual bool IsBindless() const { return true; }

  std::vector<jTextureBindData> TextureBindDatas;
};

struct jTextureArrayResourceBindless : public jShaderBindingResource {
  struct jTextureArrayBindData {
    jTexture** TextureArray  = nullptr;
    int32_t    InNumOfTexure = 0;
  };

  jTextureArrayResourceBindless() = default;

  jTextureArrayResourceBindless(
      const std::vector<jTextureArrayBindData>& InTextureArrayBindDatas)
      : TextureArrayBindDatas(InTextureArrayBindDatas) {}

  virtual ~jTextureArrayResourceBindless() {}

  virtual const void* GetResource(int32_t InIndex) const {
    return &TextureArrayBindDatas[InIndex];
  }

  virtual int32_t NumOfResource(int32_t InIndex) const {
    return TextureArrayBindDatas[InIndex].InNumOfTexure;
  }

  virtual bool IsBindless() const { return true; }

  std::vector<jTextureArrayBindData> TextureArrayBindDatas;
};

//////////////////////////////////////////////////////////////////////////

struct jShaderBindingResourceInlineAllocator {
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

struct jShaderBinding {
  static constexpr int32_t APPEND_LAST = -1;  // BindingPoint is appending last

  jShaderBinding() = default;

  jShaderBinding(const int32_t                 InBindingPoint,
                 const int32_t                 InNumOfDescriptors,
                 const EShaderBindingType      InBindingType,
                 const bool                    InIsBindless,
                 const EShaderAccessStageFlag  InAccessStageFlags,
                 const jShaderBindingResource* InResource = nullptr,
                 bool                          InIsInline = false)
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

  void CloneWithoutResource(jShaderBinding& OutReslut) const {
    OutReslut.IsInline         = IsInline;
    OutReslut.BindingPoint     = BindingPoint;
    OutReslut.NumOfDescriptors = NumOfDescriptors;
    OutReslut.BindingType      = BindingType;
    OutReslut.AccessStageFlags = AccessStageFlags;
    OutReslut.IsBindless       = IsBindless;
    OutReslut.Hash = Hash;
  }

  mutable size_t Hash = 0;

  bool                   IsInline = false;
  bool                   IsBindless       = false;
  int32_t                BindingPoint     = 0;
  int32_t                NumOfDescriptors = 1;
  EShaderBindingType     BindingType      = EShaderBindingType::UNIFORMBUFFER;
  EShaderAccessStageFlag AccessStageFlags
      = EShaderAccessStageFlag::ALL_GRAPHICS;

  const jShaderBindingResource* Resource = nullptr;
};

struct jShaderBindingArray {
  static constexpr int32_t NumOfInlineData = 10;

  template <typename... T>
  void Add(T... args) {
    static_assert(std::is_trivially_copyable<jShaderBinding>::value,
                  "jShaderBinding should be trivially copyable");

    assert(NumOfInlineData > NumOfData);
    new (&Data[NumOfData]) jShaderBinding(args...);
    ++NumOfData;
  }

  void Add(const jShaderBinding& args) {
    static_assert(std::is_trivially_copyable<jShaderBinding>::value,
                  "jShaderBinding should be trivially copyable");

    assert(NumOfInlineData > NumOfData);
    Data[NumOfData] = args;
    ++NumOfData;
  }

  size_t GetHash() const {
    size_t          Hash    = 0;
    jShaderBinding* Address = (jShaderBinding*)&Data[0];
    for (int32_t i = 0; i < NumOfData; ++i) {
      Hash ^= ((Address + i)->GetHash() << i);
    }
    return Hash;
  }

  jShaderBindingArray& operator=(const jShaderBindingArray& In) {
    memcpy(&Data[0], &In.Data[0], sizeof(jShaderBinding) * In.NumOfData);
    NumOfData = In.NumOfData;
    return *this;
  }

  const jShaderBinding* operator[](int32_t InIndex) const {
    assert(InIndex < NumOfData);
    return (jShaderBinding*)(&Data[InIndex]);
  }

  void CloneWithoutResource(jShaderBindingArray& OutResult) const {
    memcpy(&OutResult.Data[0], &Data[0], sizeof(jShaderBinding) * NumOfData);

    for (int32_t i = 0; i < NumOfData; ++i) {
      jShaderBinding* SrcAddress = (jShaderBinding*)&Data[i];
      jShaderBinding* DstAddress = (jShaderBinding*)&OutResult.Data[i];
      SrcAddress->CloneWithoutResource(*DstAddress);
    }
    OutResult.NumOfData = NumOfData;
  }

  jShaderBinding Data[NumOfInlineData];
  int32_t        NumOfData = 0;
};

template <typename T>
struct TShaderBinding : public jShaderBinding {
  TShaderBinding(const int32_t                InBindingPoint,
                 const int32_t                InNumOfDescriptors,
                 const EShaderBindingType     InBindingType,
                 const EShaderAccessStageFlag InAccessStageFlags,
                 const T&                     InData)
      : jShaderBinding(
          InBindingPoint, InNumOfDescriptors, InBindingType, InAccessStageFlags)
      , Data(InData) {}

  T Data = T();
};

// struct jShaderBindingArray {
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

enum class jShaderBindingInstanceType : uint8_t {
  SingleFrame = 0,
  MultiFrame,
  Max
};

struct jShaderBindingInstance
    : public std::enable_shared_from_this<jShaderBindingInstance> {
  virtual ~jShaderBindingInstance() {}

  const struct jShaderBindingLayout* ShaderBindingsLayouts = nullptr;

  virtual void Initialize(const jShaderBindingArray& InShaderBindingArray) {}

  virtual void UpdateShaderBindings(
      const jShaderBindingArray& InShaderBindingArray) {}

  virtual void* GetHandle() const { return nullptr; }

  virtual const std::vector<uint32_t>* GetDynamicOffsets() const {
    return nullptr;
  }

  virtual void Free() {}

  virtual jShaderBindingInstanceType GetType() const { return Type; }

  virtual void SetType(const jShaderBindingInstanceType InType) {
    Type = InType;
  }

  private:
  jShaderBindingInstanceType Type = jShaderBindingInstanceType::SingleFrame;
};

// todo : MemStack for jShaderBindingInstanceArray to allocate fast memory
using jShaderBindingInstanceArray
    = ResourceContainer<const jShaderBindingInstance*>;
using jShaderBindingInstancePtrArray
    = std::vector<std::shared_ptr<jShaderBindingInstance>>;

struct jShaderBindingLayout {
  virtual ~jShaderBindingLayout() {}

  virtual bool Initialize(const jShaderBindingArray& InShaderBindingArray) {
    return false;
  }

  virtual std::shared_ptr<jShaderBindingInstance> CreateShaderBindingInstance(
      const jShaderBindingArray&       InShaderBindingArray,
      const jShaderBindingInstanceType InType) const {
    return nullptr;
  }

  virtual size_t GetHash() const {
    if (Hash) {
      return Hash;
    }

    Hash = ShaderBindingArray.GetHash();
    return Hash;
  }

  virtual const jShaderBindingArray& GetShaderBindingsLayout() const {
    return ShaderBindingArray;
  }

  virtual void* GetHandle() const { return nullptr; }

  mutable size_t Hash = 0;

  protected:
  jShaderBindingArray ShaderBindingArray;  // Resource information is empty
};

using jShaderBindingLayoutArray
    = ResourceContainer<const jShaderBindingLayout*>;

// TODO: rename namespace + consider anonymous namespace. Also write doxygen
// comments explaining why to use this
namespace test {
uint32_t GetCurrentFrameNumber();
}  // namespace test

// To pending deallocation for MultiFrame GPU Data, Because Avoding deallocation
// inflighting GPU Data (ex. jShaderBindingInstance, IUniformBufferBlock)
template <typename T>
struct jDeallocatorMultiFrameResource {
  static constexpr int32_t NumOfFramesToWaitBeforeReleasing = 3;

  struct jPendingFreeData {
    jPendingFreeData() = default;

    jPendingFreeData(int32_t InFrameIndex, std::shared_ptr<T> InDataPtr)
        : FrameIndex(InFrameIndex)
        , DataPtr(InDataPtr) {}

    int32_t            FrameIndex = 0;
    std::shared_ptr<T> DataPtr    = nullptr;
  };

  std::vector<jPendingFreeData> PendingFree;
  int32_t                       CanReleasePendingFreeDataFrameNumber = 0;
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
          jPendingFreeData& PendingFreeData = PendingFree[i];
          if (PendingFreeData.FrameIndex < OldestFrameToKeep) {
            // Return to pending descriptor set
            if (FreeDelegate) {
              FreeDelegate(PendingFreeData.DataPtr);
            }
          } else {
            CanReleasePendingFreeDataFrameNumber
                = PendingFreeData.FrameIndex + NumOfFramesToWaitBeforeReleasing
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

    PendingFree.emplace_back(jPendingFreeData(CurrentFrameNumber, InDataPtr));
  }
};

using jDeallocatorMultiFrameShaderBindingInstance
    = jDeallocatorMultiFrameResource<jShaderBindingInstance>;
// using jDeallocatorMultiFrameUniformBufferBlock
//     = jDeallocatorMultiFrameResource<IUniformBufferBlock>;

}  // namespace game_engine

#endif  // GAME_ENGINE_SHADER_BINDING_LAYOUT_H