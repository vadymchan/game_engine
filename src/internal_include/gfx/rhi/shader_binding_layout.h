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

  virtual const void* getResource() const { return nullptr; }

  virtual int32_t numOfResource() const { return 1; }

  virtual bool isBindless() const { return false; }
};

struct UniformBufferResource : public ShaderBindingResource {
  UniformBufferResource() = default;

  UniformBufferResource(const IUniformBufferBlock* uniformBuffer)
      : m_uniformBuffer_(uniformBuffer) {}

  virtual ~UniformBufferResource() {}

  virtual const void* getResource() const override { return m_uniformBuffer_; }

  const IUniformBufferBlock* m_uniformBuffer_ = nullptr;
};

struct BufferResource : public ShaderBindingResource {
  BufferResource() = default;

  BufferResource(const Buffer* buffer)
      : m_buffer(buffer) {}

  virtual ~BufferResource() {}

  virtual const void* getResource() const override { return m_buffer; }

  const Buffer* m_buffer = nullptr;
};

struct SamplerResource : public ShaderBindingResource {
  SamplerResource() = default;

  SamplerResource(const SamplerStateInfo* samplerState)
      : m_samplerState_(samplerState) {}

  virtual ~SamplerResource() {}

  virtual const void* getResource() const override { return m_samplerState_; }

  const SamplerStateInfo* m_samplerState_ = nullptr;
};

struct TextureResource : public SamplerResource {
  TextureResource() = default;

  TextureResource(const Texture*          texture,
                  const SamplerStateInfo* samplerState,
                  int32_t                 mipLevel = 0)
      : SamplerResource(samplerState)
      , m_texture(texture)
      , m_mipLevel_(mipLevel) {}

  virtual ~TextureResource() {}

  virtual const void* getResource() const override { return m_texture; }

  const Texture* m_texture   = nullptr;
  const int32_t  m_mipLevel_ = 0;
};

struct TextureArrayResource : public ShaderBindingResource {
  TextureArrayResource() = default;

  TextureArrayResource(const Texture** textureArray,
                       const int32_t   numOfTexures)
      : m_textureArray_(textureArray)
      , m_mumOfTexures_(numOfTexures) {}

  virtual ~TextureArrayResource() {}

  virtual const void* getResource() const override { return m_textureArray_; }

  virtual int32_t numOfResource() const override { return m_mumOfTexures_; }

  const Texture** m_textureArray_ = nullptr;
  const int32_t   m_mumOfTexures_ = 1;
};

//////////////////////////////////////////////////////////////////////////

// Bindless resources, It contain multiple resources at once.
struct UniformBufferResourceBindless : public ShaderBindingResource {
  UniformBufferResourceBindless() = default;

  UniformBufferResourceBindless(
      const std::vector<const IUniformBufferBlock*>& uniformBuffers)
      : m_uniformBuffers_(uniformBuffers) {}

  virtual ~UniformBufferResourceBindless() {}

  virtual const void* getResource(int32_t index) const {
    return m_uniformBuffers_[index];
  }

  virtual int32_t getNumOfResources() const {
    return (int32_t)m_uniformBuffers_.size();
  }

  virtual bool isBindless() const { return true; }

  std::vector<const IUniformBufferBlock*> m_uniformBuffers_;
};

struct BufferResourceBindless : public ShaderBindingResource {
  BufferResourceBindless() = default;

  BufferResourceBindless(const std::vector<const Buffer*>& buffers)
      : m_buffers(buffers) {}

  virtual ~BufferResourceBindless() {}

  virtual const void* getResource(int32_t index) const {
    return m_buffers[index];
  }

  virtual bool isBindless() const { return true; }

  std::vector<const Buffer*> m_buffers;
};

struct SamplerResourceBindless : public ShaderBindingResource {
  SamplerResourceBindless() = default;

  SamplerResourceBindless(
      const std::vector<const SamplerStateInfo*>& samplerStates)
      : m_samplerStates_(samplerStates) {}

  virtual ~SamplerResourceBindless() {}

  virtual const void* getResource(int32_t index) const {
    return m_samplerStates_[index];
  }

  virtual bool isBindless() const { return true; }

  std::vector<const SamplerStateInfo*> m_samplerStates_;
};

struct TextureResourceBindless : public ShaderBindingResource {
  struct TextureBindData {
    TextureBindData() = default;

    TextureBindData(Texture*          texture,
                    SamplerStateInfo* samplerState,
                    int32_t           mipLevel = 0)
        : m_texture(texture)
        , m_samplerState_(samplerState)
        , m_mipLevel_(mipLevel) {}

    Texture*          m_texture       = nullptr;
    SamplerStateInfo* m_samplerState_ = nullptr;
    int32_t           m_mipLevel_     = 0;
  };

  TextureResourceBindless() = default;

  TextureResourceBindless(const std::vector<TextureBindData>& textureBindData)
      : m_textureBindDatas_(textureBindData) {}

  virtual ~TextureResourceBindless() {}

  virtual const void* getResource(int32_t index) const {
    return &m_textureBindDatas_[0];
  }

  virtual bool isBindless() const { return true; }

  std::vector<TextureBindData> m_textureBindDatas_;
};

struct TextureArrayResourceBindless : public ShaderBindingResource {
  struct TextureArrayBindData {
    Texture** m_textureArray_ = nullptr;
    int32_t   m_numOfTexure_  = 0;
  };

  TextureArrayResourceBindless() = default;

  TextureArrayResourceBindless(
      const std::vector<TextureArrayBindData>& textureArrayBindDatas)
      : m_textureArrayBindDatas_(textureArrayBindDatas) {}

  virtual ~TextureArrayResourceBindless() {}

  virtual const void* getResource(int32_t index) const {
    return &m_textureArrayBindDatas_[index];
  }

  virtual int32_t numOfResource(int32_t index) const {
    return m_textureArrayBindDatas_[index].m_numOfTexure_;
  }

  virtual bool isBindless() const { return true; }

  std::vector<TextureArrayBindData> m_textureArrayBindDatas_;
};

//////////////////////////////////////////////////////////////////////////

struct ShaderBindingResourceInlineAllocator {
  template <typename T, typename... T1>
  T* alloc(T1... args) {
    assert((m_offset_ + sizeof(T)) < sizeof(m_data_));

    T* AllocatedAddress  = new (&m_data_[0] + m_offset_) T(args...);
    m_offset_           += sizeof(T);
    return AllocatedAddress;
  }

  void reset() { m_offset_ = 0; }

  uint8_t m_data_[1024];
  int32_t m_offset_ = 0;
};

struct ShaderBinding {
  // TODO: rename according to the convention
  static constexpr int32_t s_kAppendLast
      = -1;  // m_bindingPoint_ is appending last

  ShaderBinding() = default;

  ShaderBinding(const int32_t                bindingPoint,
                const int32_t                numOfDescriptors,
                const EShaderBindingType     bindingType,
                const bool                   isBindless,
                const EShaderAccessStageFlag accessStageFlags,
                const ShaderBindingResource* resource = nullptr,
                bool                         isInline = false)
      : m_bindingPoint_(bindingPoint)
      , m_numOfDescriptors_(numOfDescriptors)
      , m_bindingType_(bindingType)
      , m_isBindless_(isBindless)
      , m_accessStageFlags_(accessStageFlags)
      , m_resource_(resource)
      , m_isInline_(isInline) {
    // SubpassInputAttachment must have the stageflag 0.
    assert(EShaderBindingType::SUBPASS_INPUT_ATTACHMENT != bindingType
           || accessStageFlags == EShaderAccessStageFlag::FRAGMENT);

    if (resource) {
      assert(resource->isBindless() == isBindless);
    }

    getHash();
  }

  size_t getHash() const {
    if (m_hash_) {
      return m_hash_;
    }

    m_hash_ = GETHASH_FROM_INSTANT_STRUCT(m_isInline_,
                                          m_bindingPoint_,
                                          m_numOfDescriptors_,
                                          m_bindingType_,
                                          m_accessStageFlags_,
                                          m_isBindless_);
    return m_hash_;
  }

  void cloneWithoutResource(ShaderBinding& result) const {
    result.m_isInline_         = m_isInline_;
    result.m_bindingPoint_     = m_bindingPoint_;
    result.m_numOfDescriptors_ = m_numOfDescriptors_;
    result.m_bindingType_      = m_bindingType_;
    result.m_accessStageFlags_ = m_accessStageFlags_;
    result.m_isBindless_       = m_isBindless_;
    result.m_hash_             = m_hash_;
  }

  mutable size_t m_hash_ = 0;

  bool                   m_isInline_         = false;
  bool                   m_isBindless_       = false;
  int32_t                m_bindingPoint_     = 0;
  int32_t                m_numOfDescriptors_ = 1;
  EShaderBindingType     m_bindingType_ = EShaderBindingType::UNIFORMBUFFER;
  EShaderAccessStageFlag m_accessStageFlags_
      = EShaderAccessStageFlag::ALL_GRAPHICS;

  const ShaderBindingResource* m_resource_ = nullptr;
};

struct ShaderBindingArray {
  // TODO: rename according to the convention
  static constexpr int32_t s_kNumOfInlineData = 10;

  template <typename... T>
  void add(T... args) {
    static_assert(std::is_trivially_copyable<ShaderBinding>::value,
                  "ShaderBinding should be trivially copyable");

    assert(s_kNumOfInlineData > m_numOfData_);
    new (&m_data_[m_numOfData_]) ShaderBinding(args...);
    ++m_numOfData_;
  }

  void add(const ShaderBinding& args) {
    static_assert(std::is_trivially_copyable<ShaderBinding>::value,
                  "ShaderBinding should be trivially copyable");

    assert(s_kNumOfInlineData > m_numOfData_);
    m_data_[m_numOfData_] = args;
    ++m_numOfData_;
  }

  size_t getHash() const {
    size_t         hash    = 0;
    ShaderBinding* address = (ShaderBinding*)&m_data_[0];
    for (int32_t i = 0; i < m_numOfData_; ++i) {
      hash ^= ((address + i)->getHash() << i);
    }
    return hash;
  }

  ShaderBindingArray& operator=(const ShaderBindingArray& In) {
    memcpy(
        &m_data_[0], &In.m_data_[0], sizeof(ShaderBinding) * In.m_numOfData_);
    m_numOfData_ = In.m_numOfData_;
    return *this;
  }

  const ShaderBinding* operator[](int32_t index) const {
    assert(index < m_numOfData_);
    return (ShaderBinding*)(&m_data_[index]);
  }

  void cloneWithoutResource(ShaderBindingArray& result) const {
    memcpy(&result.m_data_[0],
           &m_data_[0],
           sizeof(ShaderBinding) * m_numOfData_);

    for (int32_t i = 0; i < m_numOfData_; ++i) {
      ShaderBinding* SrcAddress = (ShaderBinding*)&m_data_[i];
      ShaderBinding* DstAddress = (ShaderBinding*)&result.m_data_[i];
      SrcAddress->cloneWithoutResource(*DstAddress);
    }
    result.m_numOfData_ = m_numOfData_;
  }

  ShaderBinding m_data_[s_kNumOfInlineData];
  int32_t       m_numOfData_ = 0;
};

template <typename T>
struct TShaderBinding : public ShaderBinding {
  TShaderBinding(const int32_t                bindingPoint,
                 const int32_t                numOfDescriptors,
                 const EShaderBindingType     bindingType,
                 const EShaderAccessStageFlag accessStageFlags,
                 const T&                     data)
      : ShaderBinding(
          bindingPoint, numOfDescriptors, bindingType, accessStageFlags)
      , m_data_(data) {}

  T m_data_ = T();
};

// struct ShaderBindingArray {
//   static constexpr int32_t NumOfInlineBindings = 10;
//
//   struct Allocator {
//     uint8* InlineStrage[NumOfInlineBindings] = {};
//
//     uint8* GetAddress() const { return InlineStrage[0]; }
//
//     void SetHeapMemeory(uint8* heap) { InlineStrage[0] = heap; }
//   };
//
//   Allocator Data{};
//   uint64_t    AllocateOffset = 0;
//
//   template <typename T>
//   void add(const TShaderBinding<T>& shaderBinding) {
//     const bool NotEnoughInlineStorage
//         = (sizeof(Data) < AllocateOffset + sizeof(shaderBinding));
//     if (NotEnoughInlineStorage) {
//       // Allocate additional memory
//       assert(0);
//     }
//
//     memcpy(Data.GetAddress() + AllocateOffset,
//            shaderBinding,
//            sizeof(shaderBinding));
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

  // TODO: check according to naming conventions
  const struct ShaderBindingLayout* m_shaderBindingsLayouts_ = nullptr;

  virtual void initialize(const ShaderBindingArray& shaderBindingArray) {}

  virtual void updateShaderBindings(
      const ShaderBindingArray& shaderBindingArray) {}

  virtual void* getHandle() const { return nullptr; }

  virtual const std::vector<uint32_t>* getDynamicOffsets() const {
    return nullptr;
  }

  virtual void free() {}

  virtual ShaderBindingInstanceType getType() const { return m_type_; }

  virtual void setType(const ShaderBindingInstanceType type) { m_type_ = type; }

  private:
  ShaderBindingInstanceType m_type_ = ShaderBindingInstanceType::SingleFrame;
};

// TODO: MemStack for ShaderBindingInstanceArray to allocate fast memory
using ShaderBindingInstanceArray
    = ResourceContainer<const ShaderBindingInstance*>;
using ShaderBindingInstancePtrArray
    = std::vector<std::shared_ptr<ShaderBindingInstance>>;

struct ShaderBindingLayout {
  virtual ~ShaderBindingLayout() {}

  virtual bool initialize(const ShaderBindingArray& shaderBindingArray) {
    return false;
  }

  virtual std::shared_ptr<ShaderBindingInstance> createShaderBindingInstance(
      const ShaderBindingArray&       shaderBindingArray,
      const ShaderBindingInstanceType type) const {
    return nullptr;
  }

  virtual size_t getHash() const {
    if (m_hash_) {
      return m_hash_;
    }

    m_hash_ = m_shaderBindingArray_.getHash();
    return m_hash_;
  }

  virtual const ShaderBindingArray& getShaderBindingsLayout() const {
    return m_shaderBindingArray_;
  }

  virtual void* getHandle() const { return nullptr; }

  mutable size_t m_hash_ = 0;

  protected:
  ShaderBindingArray m_shaderBindingArray_;  // Resource information is empty
};

using ShaderBindingLayoutArray = ResourceContainer<const ShaderBindingLayout*>;

// TODO: rename namespace + consider anonymous namespace. Also write doxygen
// comments explaining why to use this
namespace test {
uint32_t g_getCurrentFrameNumber();
}  // namespace test

// To pending deallocation for MultiFrame GPU Data, Because Avoding deallocation
// inflighting GPU Data (ex. ShaderBindingInstance, IUniformBufferBlock)
template <typename T>
struct DeallocatorMultiFrameResource {
  // TODO: check if according to naming conventions
  static constexpr int32_t s_kNumOfFramesToWaitBeforeReleasing = 3;

  struct PendingFreeData {
    PendingFreeData() = default;

    PendingFreeData(int32_t frameIndex, std::shared_ptr<T> dataPtr)
        : m_frameIndex_(frameIndex)
        , m_dataPtr_(dataPtr) {}

    int32_t            m_frameIndex_ = 0;
    std::shared_ptr<T> m_dataPtr_    = nullptr;
  };

  std::vector<PendingFreeData> m_pendingFree_;
  int32_t                      m_canReleasePendingFreeDataFrameNumber_ = 0;
  std::function<void(std::shared_ptr<T>)> m_freeDelegate_;

  void free(std::shared_ptr<T> dataPtr) {
    const int32_t CurrentFrameNumber = test::g_getCurrentFrameNumber();
    const int32_t OldestFrameToKeep
        = CurrentFrameNumber - s_kNumOfFramesToWaitBeforeReleasing;

    // ProcessPendingDescriptorPoolFree
    {
      // Check it is too early
      if (CurrentFrameNumber >= m_canReleasePendingFreeDataFrameNumber_) {
        // Release pending memory
        int32_t i = 0;
        for (; i < m_pendingFree_.size(); ++i) {
          PendingFreeData& pendingFreeData = m_pendingFree_[i];
          if (pendingFreeData.m_frameIndex_ < OldestFrameToKeep) {
            // Return to pending descriptor set
            if (m_freeDelegate_) {
              m_freeDelegate_(pendingFreeData.m_dataPtr_);
            }
          } else {
            m_canReleasePendingFreeDataFrameNumber_
                = pendingFreeData.m_frameIndex_
                + s_kNumOfFramesToWaitBeforeReleasing + 1;
            break;
          }
        }
        if (i > 0) {
          const size_t RemainingSize = (m_pendingFree_.size() - i);
          if (RemainingSize > 0) {
            for (int32_t k = 0; k < RemainingSize; ++k) {
              m_pendingFree_[k] = m_pendingFree_[i + k];
            }
          }
          m_pendingFree_.resize(RemainingSize);
        }
      }
    }

    m_pendingFree_.emplace_back(PendingFreeData(CurrentFrameNumber, dataPtr));
  }
};

using DeallocatorMultiFrameShaderBindingInstance
    = DeallocatorMultiFrameResource<ShaderBindingInstance>;
// using DeallocatorMultiFrameUniformBufferBlock
//     = DeallocatorMultiFrameResource<IUniformBufferBlock>;

}  // namespace game_engine

#endif  // GAME_ENGINE_SHADER_BINDING_LAYOUT_H