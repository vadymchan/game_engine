#ifndef GAME_ENGINE_SHADER_BINDING_LAYOUT_VK_H
#define GAME_ENGINE_SHADER_BINDING_LAYOUT_VK_H

#include "gfx/rhi/lock.h"
#include "gfx/rhi/resource_container.h"
#include "gfx/rhi/vulkan/buffer_vk.h"
// TODO: check for circular dependency
#include "gfx/rhi/i_uniform_buffer_block.h"
// #include "gfx/rhi/vulkan/pipeline_state_info_vk.h" // circular dependency
#include "gfx/rhi/vulkan/texture_vk.h"
#include "gfx/rhi/vulkan/uniform_buffer_object_vk.h"
#include "utils/third_party/xxhash_util.h"

#include <vulkan/vulkan.h>

#include <cassert>
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

namespace game_engine {

struct SamplerStateInfoVk;
struct PushConstantVk;

struct ShaderBindingResource {
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

  BufferResource(const BufferVk* InBuffer)
      : Buffer(InBuffer) {}

  virtual ~BufferResource() {}

  virtual const void* GetResource() const override { return Buffer; }

  const BufferVk* Buffer = nullptr;
};

struct SamplerResource : public ShaderBindingResource {
  SamplerResource() = default;

  SamplerResource(const SamplerStateInfoVk* InSamplerState)
      : SamplerState(InSamplerState) {}

  virtual ~SamplerResource() {}

  virtual const void* GetResource() const override { return SamplerState; }

  const SamplerStateInfoVk* SamplerState = nullptr;
};

struct TextureResource : public SamplerResource {
  TextureResource() = default;

  TextureResource(const TextureVk*          InTexture,
                  const SamplerStateInfoVk* InSamplerState,
                  int32_t                   InMipLevel = 0)
      : SamplerResource(InSamplerState)
      , Texture(InTexture)
      , MipLevel(InMipLevel) {}

  virtual ~TextureResource() {}

  virtual const void* GetResource() const override { return Texture; }

  const TextureVk* Texture  = nullptr;
  const int32_t    MipLevel = 0;
};

struct TextureArrayResource : public ShaderBindingResource {
  TextureArrayResource() = default;

  TextureArrayResource(const TextureVk** InTextureArray,
                       const int32_t     InNumOfTexure)
      : TextureArray(InTextureArray)
      , NumOfTexure(InNumOfTexure) {}

  virtual ~TextureArrayResource() {}

  virtual const void* GetResource() const override { return TextureArray; }

  virtual int32_t NumOfResource() const override { return NumOfTexure; }

  const TextureVk** TextureArray = nullptr;
  const int32_t     NumOfTexure  = 1;
};

// ========================== Bindless resources ============================
// Contains multiple resources at once.
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

  BufferResourceBindless(const std::vector<const BufferVk*>& InBuffers)
      : Buffers(InBuffers) {}

  virtual ~BufferResourceBindless() {}

  virtual const void* GetResource(int32_t InIndex) const {
    return Buffers[InIndex];
  }

  virtual bool IsBindless() const { return true; }

  std::vector<const BufferVk*> Buffers;
};

struct SamplerResourceBindless : public ShaderBindingResource {
  SamplerResourceBindless() = default;

  SamplerResourceBindless(
      const std::vector<const SamplerStateInfoVk*>& InSamplerStates)
      : SamplerStates(InSamplerStates) {}

  virtual ~SamplerResourceBindless() {}

  virtual const void* GetResource(int32_t InIndex) const {
    return SamplerStates[InIndex];
  }

  virtual bool IsBindless() const { return true; }

  std::vector<const SamplerStateInfoVk*> SamplerStates;
};

struct TextureResourceBindless : public ShaderBindingResource {
  struct TextureBindData {
    TextureBindData() = default;

    TextureBindData(TextureVk*          InTexture,
                    SamplerStateInfoVk* InSamplerState,
                    int32_t             InMipLevel = 0)
        : Texture(InTexture)
        , SamplerState(InSamplerState)
        , MipLevel(InMipLevel) {}

    TextureVk*          Texture      = nullptr;
    SamplerStateInfoVk* SamplerState = nullptr;
    int32_t             MipLevel     = 0;
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
    TextureVk** TextureArray  = nullptr;
    int32_t     InNumOfTexure = 0;
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

// ====================================================================

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

struct ShaderBindingVk {
  static constexpr int32_t APPEND_LAST = -1;  // BindingPoint is appending last

  ShaderBindingVk() = default;

  ShaderBindingVk(const int32_t                InBindingPoint,
                  const int32_t                InNumOfDescriptors,
                  const EShaderBindingType     InBindingType,
                  const EShaderAccessStageFlag InAccessStageFlags,
                  const ShaderBindingResource* InResource = nullptr,
                  bool                         InIsInline = false)
      : BindingPoint(InBindingPoint)
      , NumOfDescriptors(InNumOfDescriptors)
      , BindingType(InBindingType)
      , AccessStageFlags(InAccessStageFlags)
      , Resource(InResource)
      , IsInline(InIsInline) {
    // SubpassInputAttachment must have the stageflag 0.
    assert(EShaderBindingType::SUBPASS_INPUT_ATTACHMENT != InBindingType
           || InAccessStageFlags == EShaderAccessStageFlag::FRAGMENT);

    GetHash();
  }

  size_t GetHash() const;

  void CloneWithoutResource(ShaderBindingVk& OutReslut) const;

  mutable size_t Hash = 0;

  // TODO: no IsBindless field
  bool                   IsInline         = false;
  int32_t                BindingPoint     = 0;
  int32_t                NumOfDescriptors = 1;
  EShaderBindingType     BindingType      = EShaderBindingType::UNIFORMBUFFER;
  EShaderAccessStageFlag AccessStageFlags
      = EShaderAccessStageFlag::ALL_GRAPHICS;

  // std::shared_ptr<ShaderBindingResource> ResourcePtr;
  const ShaderBindingResource* Resource = nullptr;
};

struct ShaderBindingArray {
  static constexpr int32_t NumOfInlineData = 10;

  template <typename... T>
  void Add(T... args) {
    static_assert(std::is_trivially_copyable<ShaderBindingVk>::value,
                  "ShaderBinding should be trivially copyable");

    assert(NumOfInlineData > NumOfData);
    new (&Data[NumOfData]) ShaderBindingVk(args...);
    ++NumOfData;
  }

  size_t GetHash() const;

  ShaderBindingArray& operator=(const ShaderBindingArray& In);

  const ShaderBindingVk* operator[](int32_t InIndex) const;

  void CloneWithoutResource(ShaderBindingArray& OutResult) const;

  ShaderBindingVk Data[NumOfInlineData];
  int32_t         NumOfData = 0;
};

template <typename T>
struct TShaderBinding : public ShaderBindingVk {
  TShaderBinding(const int32_t                InBindingPoint,
                 const int32_t                InNumOfDescriptors,
                 const EShaderBindingType     InBindingType,
                 const EShaderAccessStageFlag InAccessStageFlags,
                 const T&                     InData)
      : ShaderBindingVk(
          InBindingPoint, InNumOfDescriptors, InBindingType, InAccessStageFlags)
      , Data(InData) {}

  T Data = T();
};

// struct ShaderBindingArray
//{
//     static constexpr int32_t NumOfInlineBindings = 10;
//     struct Allocator
//     {
//         uint8_t* InlineStrage[NumOfInlineBindings] = {};
//         uint8_t* GetAddress() const { return InlineStrage[0]; }
//         void SetHeapMemeory(uint8_t* InHeap) { InlineStrage[0] = InHeap; }
//     };
//     Allocator Data{};
//     uint64 AllocateOffset = 0;
//
//     template <typename T>
//     void Add(const TShaderBinding<T>& InShaderBinding)
//     {
//         const bool NotEnoughInlineStorage = (sizeof(Data) < AllocateOffset +
//         sizeof(InShaderBinding)); if (NotEnoughInlineStorage)
//         {
//             assert(0);
//         }
//
//         memcpy(Data.GetAddress() + AllocateOffset, InShaderBinding,
//         sizeof(InShaderBinding));
//     }
// };

// --------------------

struct WriteDescriptorInfo {
  WriteDescriptorInfo() = default;

  WriteDescriptorInfo(VkDescriptorBufferInfo InBufferInfo)
      : BufferInfo(InBufferInfo) {}

  WriteDescriptorInfo(VkDescriptorImageInfo InImageInfo)
      : ImageInfo(InImageInfo) {}

  // Raytracing (WIP)
  // WriteDescriptorInfo(
  //     VkWriteDescriptorSetAccelerationStructureKHR
  //     InAccelerationStructureInfo) :
  //     AccelerationStructureInfo(InAccelerationStructureInfo) {}

  VkDescriptorBufferInfo BufferInfo{};
  VkDescriptorImageInfo  ImageInfo{};
  // Raytracing (WIP)
  // VkWriteDescriptorSetAccelerationStructureKHR AccelerationStructureInfo{};
};

struct WriteDescriptorSet {
  void Reset();

  void SetWriteDescriptorInfo(int32_t                InIndex,
                              const ShaderBindingVk* InShaderBinding);

  bool                              IsInitialized = false;
  std::vector<WriteDescriptorInfo>  WriteDescriptorInfos;
  std::vector<VkWriteDescriptorSet> DescriptorWrites;
  std::vector<uint32_t>             DynamicOffsets;
};

// ----------------------

enum class ShaderBindingInstanceType : uint8_t {
  SingleFrame = 0,
  MultiFrame,
  Max
};

struct ShaderBindingInstance
    : public std::enable_shared_from_this<ShaderBindingInstance> {
  virtual ~ShaderBindingInstance() {}

  const struct ShaderBindingLayoutVk* ShaderBindingsLayouts = nullptr;

  static void CreateWriteDescriptorSet(
      WriteDescriptorSet&       OutDescriptorWrites,
      const VkDescriptorSet     InDescriptorSet,
      const ShaderBindingArray& InShaderBindingArray);

  static void UpdateWriteDescriptorSet(
      WriteDescriptorSet&       OutDescriptorWrites,
      const ShaderBindingArray& InShaderBindingArray);

  virtual void Initialize(const ShaderBindingArray& InShaderBindingArray);

  virtual void UpdateShaderBindings(
      const ShaderBindingArray& InShaderBindingArray);

  virtual void* GetHandle() const { return DescriptorSet; }

  virtual const std::vector<uint32_t>* GetDynamicOffsets() const {
    return &writeDescriptorSet.DynamicOffsets;
  }

  virtual void Free();

  virtual ShaderBindingInstanceType GetType() const { return Type; }

  virtual void SetType(const ShaderBindingInstanceType InType) {
    Type = InType;
  }

  private:
  ShaderBindingInstanceType Type = ShaderBindingInstanceType::SingleFrame;

  public:
  VkDescriptorSet    DescriptorSet = nullptr;
  WriteDescriptorSet writeDescriptorSet;
};

// todo : MemStack for ShaderBindingInstanceArray to allocate fast memory
using ShaderBindingInstanceArray
    = ResourceContainer<const ShaderBindingInstance*>;
using ShaderBindingInstancePtrArray
    = std::vector<std::shared_ptr<ShaderBindingInstance>>;

struct ShaderBindingLayoutVk;

using ShaderBindingLayoutArrayVk
    = ResourceContainer<const ShaderBindingLayoutVk*>;

struct ShaderBindingLayoutVk {
  virtual ~ShaderBindingLayoutVk() { Release(); }

  virtual bool Initialize(const ShaderBindingArray& InShaderBindingArray);

  virtual std::shared_ptr<ShaderBindingInstance> CreateShaderBindingInstance(
      const ShaderBindingArray&       InShaderBindingArray,
      const ShaderBindingInstanceType InType) const;

  virtual size_t GetHash() const;

  virtual const ShaderBindingArray& GetShaderBindingsLayout() const {
    return shaderBindingArray;
  }

  virtual void* GetHandle() const { return DescriptorSetLayout; }

  void Release();

  std::vector<VkDescriptorPoolSize> GetDescriptorPoolSizeArray(
      uint32_t maxAllocations) const;

  mutable size_t Hash = 0;

  protected:
  ShaderBindingArray shaderBindingArray;

  public:
  VkDescriptorSetLayout DescriptorSetLayout = nullptr;

  static VkDescriptorSetLayout CreateDescriptorSetLayout(
      const ShaderBindingArray& InShaderBindingArray);

  static VkPipelineLayout CreatePipelineLayout(
      const ShaderBindingLayoutArrayVk& InShaderBindingLayoutArray,
      const PushConstantVk*             pushConstant);

  static void ClearPipelineLayout();

  static MutexRWLock DescriptorLayoutPoolLock;
  static std::unordered_map<size_t, VkDescriptorSetLayout> DescriptorLayoutPool;

  static MutexRWLock                                  PipelineLayoutPoolLock;
  static std::unordered_map<size_t, VkPipelineLayout> PipelineLayoutPool;
};

// TODO: rename namespace + consider anonymous namespace. Also write doxygen
// comments explaining why to use this
namespace test {
uint32_t GetCurrentFrameNumber();
}  // namespace test

// To pending deallocation for MultiFrame GPU Data, Because Avoding
// deallocation inflighting GPU Data (ex. ShaderBindingInstance,
// IUniformBufferBlock)
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
          PendingFreeData& PendingFreeData = PendingFree[i];
          if (PendingFreeData.FrameIndex < OldestFrameToKeep) {
            // Return to pending descriptor set
            assert(PendingFreeData.DataPtr);
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

    PendingFree.emplace_back(PendingFreeData(CurrentFrameNumber, InDataPtr));
  }
};

using DeallocatorMultiFrameShaderBindingInstance
    = DeallocatorMultiFrameResource<ShaderBindingInstance>;
using DeallocatorMultiFrameUniformBufferBlock
    = DeallocatorMultiFrameResource<UniformBufferBlockVk>;

}  // namespace game_engine

#endif  // GAME_ENGINE_SHADER_BINDING_LAYOUT_VK_H