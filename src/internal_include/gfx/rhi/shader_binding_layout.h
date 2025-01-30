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
  // ======= BEGIN: public destructor =========================================

  virtual ~ShaderBindingResource() {}

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overridden methods =================================

  virtual int32_t numOfResource() const { return 1; }

  virtual bool isBindless() const { return false; }

  virtual const void* getResource() const { return nullptr; }

  // ======= END: public overridden methods   =================================
};

struct UniformBufferResource : public ShaderBindingResource {
  // ======= BEGIN: public constructors =======================================

  UniformBufferResource() = default;

  UniformBufferResource(const IUniformBufferBlock* uniformBuffer)
      : m_uniformBuffer_(uniformBuffer) {}

  // ======= END: public constructors   =======================================

  // ======= BEGIN: public destructor =========================================

  virtual ~UniformBufferResource() {}

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overridden methods =================================

  virtual const void* getResource() const override { return m_uniformBuffer_; }

  // ======= END: public overridden methods   =================================

  // ======= BEGIN: public constants ==========================================

  const IUniformBufferBlock* m_uniformBuffer_ = nullptr;

  // ======= END: public constants   ==========================================
};

struct BufferResource : public ShaderBindingResource {
  // ======= BEGIN: public constructors =======================================

  BufferResource() = default;

  BufferResource(const IBuffer* buffer)
      : m_buffer(buffer) {}

  // ======= END: public constructors   =======================================

  // ======= BEGIN: public destructor =========================================

  virtual ~BufferResource() {}

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overridden methods =================================

  virtual const void* getResource() const override { return m_buffer; }

  // ======= END: public overridden methods   =================================

  // ======= BEGIN: public constants ==========================================

  const IBuffer* m_buffer = nullptr;

  // ======= END: public constants   ==========================================
};

struct SamplerResource : public ShaderBindingResource {
  // ======= BEGIN: public constructors =======================================

  SamplerResource() = default;

  SamplerResource(const SamplerStateInfo* samplerState)
      : m_samplerState_(samplerState) {}

  // ======= END: public constructors   =======================================

  // ======= BEGIN: public destructor =========================================

  virtual ~SamplerResource() {}

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overridden methods =================================

  virtual const void* getResource() const override { return m_samplerState_; }

  // ======= END: public overridden methods   =================================

  // ======= BEGIN: public constants ==========================================

  const SamplerStateInfo* m_samplerState_ = nullptr;

  // ======= END: public constants   ==========================================
};

struct TextureResource : public SamplerResource {
  // ======= BEGIN: public constructors =======================================

  TextureResource() = default;

  TextureResource(Texture*                texture,
                  const SamplerStateInfo* samplerState,
                  int32_t                 mipLevel = 0)
      : SamplerResource(samplerState)
      , m_texture_(texture)
      , kMipLevel(mipLevel) {}

  // ======= END: public constructors   =======================================

  // ======= BEGIN: public destructor =========================================

  virtual ~TextureResource() {}

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overridden methods =================================

  virtual const void* getResource() const override { return m_texture_; }

  // ======= END: public overridden methods   =================================

  // ======= BEGIN: public constants ==========================================

  Texture*      m_texture_ = nullptr;
  const int32_t kMipLevel  = 0;

  // ======= END: public constants   ==========================================
};

struct TextureArrayResource : public ShaderBindingResource {
  // ======= BEGIN: public constructors =======================================

  TextureArrayResource() = default;

  TextureArrayResource(const Texture** textureArray, const int32_t numOfTexures)
      : kTextureArray(textureArray)
      , kNumOfTexures(numOfTexures) {}

  // ======= END: public constructors   =======================================

  // ======= BEGIN: public destructor =========================================

  virtual ~TextureArrayResource() {}

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overridden methods =================================

  virtual int32_t numOfResource() const override { return kNumOfTexures; }

  virtual const void* getResource() const override { return kTextureArray; }

  // ======= END: public overridden methods   =================================

  // ======= BEGIN: public constants ==========================================

  const Texture** kTextureArray = nullptr;
  const int32_t   kNumOfTexures = 1;

  // ======= END: public constants   ==========================================
};

//////////////////////////////////////////////////////////////////////////

// Bindless resources, It contain multiple resources at once.
struct UniformBufferResourceBindless : public ShaderBindingResource {
  // ======= BEGIN: public constructors =======================================

  UniformBufferResourceBindless() = default;

  UniformBufferResourceBindless(
      const std::vector<const IUniformBufferBlock*>& uniformBuffers)
      : m_uniformBuffers_(uniformBuffers) {}

  // ======= END: public constructors   =======================================

  // ======= BEGIN: public destructor =========================================

  virtual ~UniformBufferResourceBindless() {}

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overridden methods =================================

  virtual bool isBindless() const { return true; }

  virtual const void* getResource(int32_t index) const {
    return m_uniformBuffers_[index];
  }

  virtual int32_t getNumOfResources() const {
    return (int32_t)m_uniformBuffers_.size();
  }

  // ======= END: public overridden methods   =================================

  // ======= BEGIN: public misc fields ========================================

  std::vector<const IUniformBufferBlock*> m_uniformBuffers_;

  // ======= END: public misc fields   ========================================
};

struct BufferResourceBindless : public ShaderBindingResource {
  // ======= BEGIN: public constructors =======================================

  BufferResourceBindless() = default;

  BufferResourceBindless(const std::vector<const IBuffer*>& buffers)
      : m_buffers(buffers) {}

  // ======= END: public constructors   =======================================

  // ======= BEGIN: public destructor =========================================

  virtual ~BufferResourceBindless() {}

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overridden methods =================================

  virtual bool isBindless() const { return true; }

  virtual const void* getResource(int32_t index) const {
    return m_buffers[index];
  }

  // ======= END: public overridden methods   =================================

  // ======= BEGIN: public misc fields ========================================

  std::vector<const IBuffer*> m_buffers;

  // ======= END: public misc fields   ========================================
};

struct SamplerResourceBindless : public ShaderBindingResource {
  // ======= BEGIN: public constructors =======================================

  SamplerResourceBindless() = default;

  SamplerResourceBindless(
      const std::vector<const SamplerStateInfo*>& samplerStates)
      : m_samplerStates_(samplerStates) {}

  // ======= END: public constructors   =======================================

  // ======= BEGIN: public destructor =========================================

  virtual ~SamplerResourceBindless() {}

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overridden methods =================================

  virtual const void* getResource(int32_t index) const {
    return m_samplerStates_[index];
  }

  virtual bool isBindless() const { return true; }

  // ======= END: public overridden methods   =================================

  // ======= BEGIN: public misc fields ========================================

  std::vector<const SamplerStateInfo*> m_samplerStates_;

  // ======= END: public misc fields   ========================================
};

struct TextureResourceBindless : public ShaderBindingResource {
  // ======= BEGIN: public nested types =======================================

  struct TextureBindData {
    TextureBindData() = default;

    TextureBindData(std::shared_ptr<Texture> texture,
                    SamplerStateInfo*        samplerState,
                    int32_t                  mipLevel = 0)
        : m_texture(std::move(texture))
        , m_samplerState_(samplerState)
        , m_mipLevel_(mipLevel) {}

    std::shared_ptr<Texture> m_texture       = nullptr;
    SamplerStateInfo*        m_samplerState_ = nullptr;
    int32_t                  m_mipLevel_     = 0;
  };

  // ======= END: public nested types   =======================================

  // ======= BEGIN: public constructors =======================================

  TextureResourceBindless() = default;

  TextureResourceBindless(const std::vector<TextureBindData>& textureBindData)
      : m_textureBindDatas_(textureBindData) {}

  // ======= END: public constructors   =======================================

  // ======= BEGIN: public destructor =========================================

  virtual ~TextureResourceBindless() {}

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overridden methods =================================

  virtual const void* getResource(int32_t index) const {
    return &m_textureBindDatas_[0];
  }

  virtual bool isBindless() const { return true; }

  // ======= END: public overridden methods   =================================

  // ======= BEGIN: public misc fields ========================================

  std::vector<TextureBindData> m_textureBindDatas_;

  // ======= END: public misc fields   ========================================
};

struct TextureArrayResourceBindless : public ShaderBindingResource {
  // ======= BEGIN: public nested types =======================================

  struct TextureArrayBindData {
    Texture** m_textureArray_ = nullptr;
    int32_t   m_numOfTexure_  = 0;
  };

  // ======= END: public nested types   =======================================

  // ======= BEGIN: public constructors =======================================

  TextureArrayResourceBindless() = default;

  TextureArrayResourceBindless(
      const std::vector<TextureArrayBindData>& textureArrayBindDatas)
      : m_textureArrayBindDatas_(textureArrayBindDatas) {}

  // ======= END: public constructors   =======================================

  // ======= BEGIN: public destructor =========================================

  virtual ~TextureArrayResourceBindless() {}

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overridden methods =================================

  virtual int32_t numOfResource(int32_t index) const {
    return m_textureArrayBindDatas_[index].m_numOfTexure_;
  }

  virtual bool isBindless() const { return true; }

  virtual const void* getResource(int32_t index) const {
    return &m_textureArrayBindDatas_[index];
  }

  // ======= END: public overridden methods   =================================

  // ======= BEGIN: public misc fields ========================================

  std::vector<TextureArrayBindData> m_textureArrayBindDatas_;

  // ======= END: public misc fields   ========================================
};

//////////////////////////////////////////////////////////////////////////

struct ShaderBindingResourceInlineAllocator {
  // ======= BEGIN: public misc methods =======================================

  template <typename T, typename... T1>
  T* alloc(T1... args) {
    assert((m_offset_ + sizeof(T)) < sizeof(m_data_));

    T* AllocatedAddress  = new (&m_data_[0] + m_offset_) T(args...);
    m_offset_           += sizeof(T);
    return AllocatedAddress;
  }

  void reset() { m_offset_ = 0; }

  // ======= END: public misc methods   =======================================

  // ======= BEGIN: public misc fields ========================================

  uint8_t m_data_[1024];
  int32_t m_offset_ = 0;

  // ======= END: public misc fields   ========================================
};

struct ShaderBinding {
  // ======= BEGIN: public constructors =======================================

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

  // ======= END: public constructors   =======================================

  // ======= BEGIN: public getters ============================================

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

  // ======= END: public getters   ============================================

  // ======= BEGIN: public misc methods =======================================

  void cloneWithoutResource(ShaderBinding& result) const {
    result.m_isInline_         = m_isInline_;
    result.m_bindingPoint_     = m_bindingPoint_;
    result.m_numOfDescriptors_ = m_numOfDescriptors_;
    result.m_bindingType_      = m_bindingType_;
    result.m_accessStageFlags_ = m_accessStageFlags_;
    result.m_isBindless_       = m_isBindless_;
    result.m_hash_             = m_hash_;
  }

  // ======= END: public misc methods   =======================================

  // ======= BEGIN: public constants ==========================================

  // TODO: rename according to the convention
  // m_bindingPoint_ is appending last
  static constexpr int32_t s_kAppendLast = -1;

  // ======= END: public constants   ==========================================

  // ======= BEGIN: public misc fields ========================================

  mutable size_t m_hash_ = 0;

  bool                   m_isInline_         = false;
  bool                   m_isBindless_       = false;
  int32_t                m_bindingPoint_     = 0;
  int32_t                m_numOfDescriptors_ = 1;  // TODO:: Seems like always 1
  EShaderBindingType     m_bindingType_ = EShaderBindingType::UNIFORMBUFFER;
  EShaderAccessStageFlag m_accessStageFlags_
      = EShaderAccessStageFlag::ALL_GRAPHICS;

  const ShaderBindingResource* m_resource_ = nullptr;

  // ======= END: public misc fields   ========================================
};

struct ShaderBindingArray {
  // ======= BEGIN: public misc methods =======================================

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

  void cloneWithoutResource(ShaderBindingArray& result) const {
    memcpy(
        &result.m_data_[0], &m_data_[0], sizeof(ShaderBinding) * m_numOfData_);

    for (int32_t i = 0; i < m_numOfData_; ++i) {
      ShaderBinding* srcAddress = (ShaderBinding*)&m_data_[i];
      ShaderBinding* dstAddress = (ShaderBinding*)&result.m_data_[i];
      srcAddress->cloneWithoutResource(*dstAddress);
    }
    result.m_numOfData_ = m_numOfData_;
  }

  // ======= END: public misc methods   =======================================

  // ======= BEGIN: public getters ============================================

  size_t getHash() const {
    size_t         hash    = 0;
    ShaderBinding* address = (ShaderBinding*)&m_data_[0];
    for (int32_t i = 0; i < m_numOfData_; ++i) {
      hash ^= ((address + i)->getHash() << i);
    }
    return hash;
  }

  // ======= END: public getters   ============================================

  // ======= BEGIN: public overloaded operators ===============================

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

  // ======= END: public overloaded operators   ===============================

  // ======= BEGIN: public constants ==========================================

  // TODO: rename according to the convention
  static constexpr int32_t s_kNumOfInlineData = 10;

  // ======= END: public constants   ==========================================

  // ======= BEGIN: public misc fields ========================================

  ShaderBinding m_data_[s_kNumOfInlineData];
  int32_t       m_numOfData_ = 0;

  // ======= END: public misc fields   ========================================
};

template <typename T>
struct TShaderBinding : public ShaderBinding {
  // ======= BEGIN: public constructors =======================================

  // TODO: currently isBindless is always false
  TShaderBinding(const int32_t                bindingPoint,
                 const int32_t                numOfDescriptors,
                 const EShaderBindingType     bindingType,
                 const EShaderAccessStageFlag accessStageFlags,
                 const T&                     data)
      : ShaderBinding(bindingPoint,
                      numOfDescriptors,
                      bindingType,
                      false,
                      accessStageFlags)
      , m_data_(data) {}

  // ======= END: public constructors   =======================================

  // ======= BEGIN: public misc fields ========================================

  T m_data_ = T();

  // ======= END: public misc fields   ========================================
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

struct ShaderBindingLayout;

struct ShaderBindingInstance
    : public std::enable_shared_from_this<ShaderBindingInstance> {
  // ======= BEGIN: public destructor =========================================

  virtual ~ShaderBindingInstance() {}

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overridden methods =================================

  virtual void initialize(const ShaderBindingArray& shaderBindingArray) {}

  virtual void updateShaderBindings(
      const ShaderBindingArray& shaderBindingArray) {}

  virtual void free() {}

  virtual void* getHandle() const { return nullptr; }

  virtual const std::vector<uint32_t>* getDynamicOffsets() const {
    return nullptr;
  }

  virtual ShaderBindingInstanceType getType() const { return m_type_; }

  virtual void setType(const ShaderBindingInstanceType type) { m_type_ = type; }

  // ======= END: public overridden methods   =================================

  // ======= BEGIN: public misc fields ========================================

  // TODO: check according to naming conventions
  std::shared_ptr<ShaderBindingLayout> m_shaderBindingsLayouts_ = nullptr;

  // ======= END: public misc fields   ========================================

  private:
  // ======= BEGIN: private misc fields =======================================

  ShaderBindingInstanceType m_type_ = ShaderBindingInstanceType::SingleFrame;

  // ======= END: private misc fields   =======================================
};

// TODO: consider moving somewhere else
// TODO: MemStack for ShaderBindingInstanceArray to allocate fast memory
using ShaderBindingInstanceArray
    = ResourceContainer<const ShaderBindingInstance*>;
using ShaderBindingInstancePtrArray
    = std::vector<std::shared_ptr<ShaderBindingInstance>>;

struct ShaderBindingLayout
    : public std::enable_shared_from_this<ShaderBindingLayout> {
  // ======= BEGIN: public destructor =========================================

  virtual ~ShaderBindingLayout() {}

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overridden methods =================================

  virtual bool initialize(const ShaderBindingArray& shaderBindingArray) {
    return false;
  }

  virtual std::shared_ptr<ShaderBindingInstance> createShaderBindingInstance(
      const ShaderBindingArray&       shaderBindingArray,
      const ShaderBindingInstanceType type) {
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

  // ======= END: public overridden methods   =================================

  // ======= BEGIN: public misc fields ========================================

  mutable size_t m_hash_ = 0;

  // ======= END: public misc fields   ========================================

  protected:
  // ======= BEGIN: protected misc fields =====================================

  ShaderBindingArray m_shaderBindingArray_;  // Resource information is empty

  // ======= END: protected misc fields   =====================================
};

// TODO: consider moving to other place
using ShaderBindingLayoutArray = ResourceContainer<const ShaderBindingLayout*>;

// TODO: move to other place. rename namespace + consider anonymous namespace.
// Also write doxygen comments explaining why to use this
namespace test {
uint32_t g_getCurrentFrameNumber();
}  // namespace test

// To pending deallocation for MultiFrame GPU Data, Because Avoding deallocation
// inflighting GPU Data (ex. ShaderBindingInstance, IUniformBufferBlock)
template <typename T>
struct DeallocatorMultiFrameResource {
  // ======= BEGIN: public nested types =======================================

  struct PendingFreeData {
    PendingFreeData() = default;

    PendingFreeData(int32_t frameIndex, std::shared_ptr<T> dataPtr)
        : m_frameIndex_(frameIndex)
        , m_dataPtr_(dataPtr) {}

    int32_t            m_frameIndex_ = 0;
    std::shared_ptr<T> m_dataPtr_    = nullptr;
  };

  // ======= END: public nested types   =======================================

  // ======= BEGIN: public misc methods =======================================

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

  // ======= END: public misc methods   =======================================

  // ======= BEGIN: public constants ==========================================

  // TODO: check if according to naming conventions
  static constexpr int32_t s_kNumOfFramesToWaitBeforeReleasing = 3;

  // ======= END: public constants   ==========================================

  // ======= BEGIN: public misc fields ========================================

  std::vector<PendingFreeData> m_pendingFree_;
  int32_t                      m_canReleasePendingFreeDataFrameNumber_ = 0;
  std::function<void(std::shared_ptr<T>)> m_freeDelegate_;

  // ======= END: public misc fields   ========================================
};

// TODO: consider moving somewhere else
using DeallocatorMultiFrameShaderBindingInstance
    = DeallocatorMultiFrameResource<ShaderBindingInstance>;
// using DeallocatorMultiFrameUniformBufferBlock
//     = DeallocatorMultiFrameResource<IUniformBufferBlock>;

}  // namespace game_engine

#endif  // GAME_ENGINE_SHADER_BINDING_LAYOUT_H
