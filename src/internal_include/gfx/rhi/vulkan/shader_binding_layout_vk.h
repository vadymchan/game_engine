#ifndef GAME_ENGINE_SHADER_BINDING_LAYOUT_VK_H
#define GAME_ENGINE_SHADER_BINDING_LAYOUT_VK_H

#include "gfx/rhi/i_uniform_buffer_block.h"
#include "gfx/rhi/lock.h"
#include "gfx/rhi/resource_container.h"
#include "gfx/rhi/shader_binding_layout.h"
#include "gfx/rhi/vulkan/buffer_vk.h"
// #include "gfx/rhi/vulkan/pipeline_state_info_vk.h" // circular dependency
#include "gfx/rhi/pipeline_state_info.h"  // circular dependency
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

struct WriteDescriptorInfo {
  // ======= BEGIN: public constructors =======================================

  WriteDescriptorInfo() = default;

  WriteDescriptorInfo(VkDescriptorBufferInfo bufferInfo)
      : m_bufferInfo_(bufferInfo) {}

  WriteDescriptorInfo(VkDescriptorImageInfo imageInfo)
      : m_imageInfo_(imageInfo) {}

  // Raytracing (WIP)
  // WriteDescriptorInfo(
  //     VkWriteDescriptorSetAccelerationStructureKHR
  //     accelerationStructureInfo) :
  //     AccelerationStructureInfo(accelerationStructureInfo) {}

  // ======= END: public constructors   =======================================

  // ======= BEGIN: public misc fields ========================================

  VkDescriptorBufferInfo m_bufferInfo_{};
  VkDescriptorImageInfo  m_imageInfo_{};
  // Raytracing (WIP)
  // VkWriteDescriptorSetAccelerationStructureKHR
  // m_accelerationStructureInfo_{};

  // ======= END: public misc fields   ========================================
};

struct WriteDescriptorSet {
  // ======= BEGIN: public misc methods =======================================

  void reset();

  void setWriteDescriptorInfo(int32_t              index,
                              const ShaderBinding* shaderBinding);

  // ======= END: public misc methods   =======================================

  // ======= BEGIN: public misc fields ========================================

  bool                              m_isInitialized_ = false;
  std::vector<WriteDescriptorInfo>  m_writeDescriptorInfos_;
  // This is the final result, generated using WriteDescriptorInfos
  std::vector<VkWriteDescriptorSet> m_descriptorWrites_;
  std::vector<uint32_t>             m_dynamicOffsets_;

  // ======= END: public misc fields   ========================================
};

struct ShaderBindingInstanceVk : public ShaderBindingInstance {
  // ======= BEGIN: public static methods =====================================

  static void s_createWriteDescriptorSet(
      WriteDescriptorSet&       descriptorWrites,
      const VkDescriptorSet     descriptorSet,
      const ShaderBindingArray& shaderBindingArray);

  static void s_updateWriteDescriptorSet(
      WriteDescriptorSet&       descriptorWrites,
      const ShaderBindingArray& shaderBindingArray);

  // ======= END: public static methods   =====================================

  // ======= BEGIN: public destructor =========================================

  virtual ~ShaderBindingInstanceVk() {}

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overridden methods =================================

  virtual void initialize(
      const ShaderBindingArray& shaderBindingArray) override;

  virtual void updateShaderBindings(
      const ShaderBindingArray& shaderBindingArray) override;

  virtual void free() override;

  virtual void* getHandle() const override { return m_descriptorSet_; }

  virtual const std::vector<uint32_t>* getDynamicOffsets() const override {
    return &m_writeDescriptorSet_.m_dynamicOffsets_;
  }

  virtual ShaderBindingInstanceType getType() const override { return m_type_; }

  virtual void setType(const ShaderBindingInstanceType type) override {
    m_type_ = type;
  }

  // ======= END: public overridden methods   =================================

  // ======= BEGIN: public misc fields ========================================

  // When the DescriptorPool is released, everything can be handled, so it is
  // not separately destroyed
  VkDescriptorSet    m_descriptorSet_ = nullptr;
  WriteDescriptorSet m_writeDescriptorSet_;

  // ======= END: public misc fields   ========================================

  private:
  // ======= BEGIN: private misc fields =======================================

  ShaderBindingInstanceType m_type_ = ShaderBindingInstanceType::SingleFrame;

  // ======= END: private misc fields   =======================================
};

struct ShaderBindingLayoutVk : public ShaderBindingLayout {
  // ======= BEGIN: public static methods =====================================

  static VkDescriptorSetLayout s_createDescriptorSetLayout(
      const ShaderBindingArray& shaderBindingArray);

  static VkPipelineLayout s_createPipelineLayout(
      const ShaderBindingLayoutArray& shaderBindingLayoutArray,
      const PushConstant*             pushConstant);

  static void s_clearPipelineLayout();

  // ======= END: public static methods   =====================================

  // ======= BEGIN: public static fields ======================================

  static MutexRWLock s_descriptorLayoutPoolLock;
  static std::unordered_map<size_t, VkDescriptorSetLayout>
      s_descriptorLayoutPool;

  static MutexRWLock                                  s_pipelineLayoutPoolLock;
  static std::unordered_map<size_t, VkPipelineLayout> s_pipelineLayoutPool;

  // ======= END: public static fields   ======================================

  // ======= BEGIN: public destructor =========================================

  virtual ~ShaderBindingLayoutVk() { release(); }

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overridden methods =================================

  virtual bool initialize(
      const ShaderBindingArray& shaderBindingArray) override;

  virtual std::shared_ptr<ShaderBindingInstance> createShaderBindingInstance(
      const ShaderBindingArray&       shaderBindingArray,
      const ShaderBindingInstanceType type) override;

  virtual size_t getHash() const override;

  virtual const ShaderBindingArray& getShaderBindingsLayout() const {
    return m_shaderBindingArray_;
  }

  virtual void* getHandle() const override { return m_descriptorSetLayout_; }

  // ======= END: public overridden methods   =================================

  // ======= BEGIN: protected getters =========================================

  // TODO: not used
  std::vector<VkDescriptorPoolSize> getDescriptorPoolSizeArray(
      uint32_t maxAllocations) const;

  // ======= END: protected getters   =========================================

  // ======= BEGIN: public misc methods =======================================

  void release();

  // ======= END: public misc methods   =======================================

  // ======= BEGIN: public misc fields ========================================

  mutable size_t        m_hash_                = 0;
  VkDescriptorSetLayout m_descriptorSetLayout_ = nullptr;

  // ======= END: public misc fields   ========================================

  protected:
  // ======= BEGIN: protected misc fields =====================================

  ShaderBindingArray m_shaderBindingArray_;

  // ======= END: protected misc fields   =====================================
};

}  // namespace game_engine

#endif  // GAME_ENGINE_SHADER_BINDING_LAYOUT_VK_H
