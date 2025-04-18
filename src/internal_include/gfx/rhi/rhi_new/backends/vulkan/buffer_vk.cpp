#include "gfx/rhi/rhi_new/backends/vulkan/buffer_vk.h"

#include "gfx/rhi/rhi_new/backends/vulkan/device_vk.h"
#include "gfx/rhi/rhi_new/backends/vulkan/rhi_enums_vk.h"
#include "utils/logger/global_logger.h"

namespace game_engine {
namespace gfx {
namespace rhi {

BufferVk::BufferVk(const BufferDesc& desc, DeviceVk* device)
    : Buffer(desc)
    , m_device_(device) {
  // Determine usage flags and memory based on buffer type and creation flags
  VkBufferUsageFlags    usage    = getBufferUsageFlags_();
  VkMemoryPropertyFlags memProps = getMemoryPropertyFlags_();

  if (!createBuffer_(usage, memProps)) {
    GlobalLogger::Log(LogLevel::Error, "Failed to create Vulkan buffer");
    return;
  }

  // If this is a CPU-accessible and dynamic buffer, we can keep it permanently mapped
  if ((memProps & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
      && (m_desc_.type == BufferType::Dynamic
          || ((m_desc_.createFlags & BufferCreateFlag::CpuAccess) != BufferCreateFlag::None))) {
    if (vkMapMemory(m_device_->getDevice(), m_memory_, 0, m_desc_.size, 0, &m_mappedData_) == VK_SUCCESS) {
      m_isMapped_ = true;
    } else {
      GlobalLogger::Log(LogLevel::Warning, "Failed to persistently map Vulkan buffer");
    }
  }
}

BufferVk::~BufferVk() {
  // Unmap if mapped
  if (m_isMapped_) {
    vkUnmapMemory(m_device_->getDevice(), m_memory_);
    m_mappedData_ = nullptr;
    m_isMapped_   = false;
  }

  // Destroy buffer and free memory
  if (m_buffer_ != VK_NULL_HANDLE) {
    vkDestroyBuffer(m_device_->getDevice(), m_buffer_, nullptr);
    m_buffer_ = VK_NULL_HANDLE;
  }

  if (m_memory_ != VK_NULL_HANDLE) {
    vkFreeMemory(m_device_->getDevice(), m_memory_, nullptr);
    m_memory_ = VK_NULL_HANDLE;
  }
}

bool BufferVk::update_(const void* data, size_t size, size_t offset) {
  if (!data || size == 0) {
    return false;
  }

  // Check that size + offset does not exceed the buffer size
  if (offset + size > m_desc_.size) {
    GlobalLogger::Log(LogLevel::Error, "Buffer update exceeds buffer size");
    return false;
  }

  // Check if the buffer is permanently mapped
  if (m_isMapped_ && m_mappedData_) {
    // Copy data directly into the mapped memory
    char* mappedCharPtr = static_cast<char*>(m_mappedData_);
    memcpy(mappedCharPtr + offset, data, size);

    // Ensure the writes are visible to the GPU
    // For non-coherent memory, we need to flush
    if (!(getMemoryPropertyFlags_() & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
      VkMappedMemoryRange memRange = {};
      memRange.sType               = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
      memRange.memory              = m_memory_;
      memRange.offset              = offset;
      memRange.size                = size;
      vkFlushMappedMemoryRanges(m_device_->getDevice(), 1, &memRange);
    }
    return true;
  }

  // For other buffer types, Device will use a staging buffer
  // and command buffers, but that logic is in DeviceVk::updateBuffer
  return false;
}

bool BufferVk::map_(void** ppData) {
  if (!ppData) {
    return false;
  }

  if (m_isMapped_) {
    *ppData = m_mappedData_;
    return true;
  }

  // Check if the buffer is mappable
  VkMemoryPropertyFlags memProps = getMemoryPropertyFlags_();
  if (!(memProps & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
    GlobalLogger::Log(LogLevel::Error, "Cannot map a non-host-visible buffer");
    return false;
  }

  if (vkMapMemory(m_device_->getDevice(), m_memory_, 0, m_desc_.size, 0, ppData) != VK_SUCCESS) {
    GlobalLogger::Log(LogLevel::Error, "Failed to map buffer memory");
    return false;
  }

  m_mappedData_ = *ppData;
  m_isMapped_   = true;
  return true;
}

void BufferVk::unmap_() {
  if (!m_isMapped_) {
    return;
  }

  vkUnmapMemory(m_device_->getDevice(), m_memory_);
  m_mappedData_ = nullptr;
  m_isMapped_   = false;
}

bool BufferVk::createBuffer_(VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) {
  // Create the buffer
  VkBufferCreateInfo bufferInfo = {};
  bufferInfo.sType              = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size               = m_desc_.size;
  bufferInfo.usage              = usage;
  bufferInfo.sharingMode        = VK_SHARING_MODE_EXCLUSIVE;

  if (vkCreateBuffer(m_device_->getDevice(), &bufferInfo, nullptr, &m_buffer_) != VK_SUCCESS) {
    return false;
  }

  // Retrieve memory requirements
  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(m_device_->getDevice(), m_buffer_, &memRequirements);

  // Allocate memory
  VkMemoryAllocateInfo allocInfo = {};
  allocInfo.sType                = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize       = memRequirements.size;
  allocInfo.memoryTypeIndex      = findMemoryType_(memRequirements.memoryTypeBits, properties);

  if (vkAllocateMemory(m_device_->getDevice(), &allocInfo, nullptr, &m_memory_) != VK_SUCCESS) {
    vkDestroyBuffer(m_device_->getDevice(), m_buffer_, nullptr);
    m_buffer_ = VK_NULL_HANDLE;
    return false;
  }

  // Bind the memory to the buffer
  vkBindBufferMemory(m_device_->getDevice(), m_buffer_, m_memory_, 0);

  return true;
}

VkBufferUsageFlags BufferVk::getBufferUsageFlags_() const {
  VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;  // Always allow as copy destination

  // Add transfer source bit for any buffer that can be read back
  if ((m_desc_.createFlags & BufferCreateFlag::Readback) != BufferCreateFlag::None) {
    usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  }

  // Add specific usage flags based on creation flags
  if ((m_desc_.createFlags & BufferCreateFlag::VertexBuffer) != BufferCreateFlag::None) {
    usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
  }

  if ((m_desc_.createFlags & BufferCreateFlag::IndexBuffer) != BufferCreateFlag::None) {
    usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
  }

  if ((m_desc_.createFlags & BufferCreateFlag::Uav) != BufferCreateFlag::None) {
    usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
  }

  if ((m_desc_.createFlags & BufferCreateFlag::IndirectCommand) != BufferCreateFlag::None) {
    usage |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
  }

  if ((m_desc_.createFlags & BufferCreateFlag::AccelerationStructure) != BufferCreateFlag::None) {
    usage |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
  }

  if ((m_desc_.createFlags & BufferCreateFlag::AccelerationStructureBuildInput) != BufferCreateFlag::None) {
    usage |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
  }

  if ((m_desc_.createFlags & BufferCreateFlag::ShaderBindingTable) != BufferCreateFlag::None) {
    usage |= VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR;
  }

  // If no specific flags are set, assume it's a uniform buffer
  if (!(usage
        & (VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
           | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT))) {
    usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
  }

  return usage;
}

VkMemoryPropertyFlags BufferVk::getMemoryPropertyFlags_() const {
  VkMemoryPropertyFlags props = 0;

  // Determine memory properties based on buffer type
  if (m_desc_.type == BufferType::Static) {
    // Static buffers are typically in device-local (GPU) memory
    props = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    // If CpuAccess is set, make it host visible and coherent
    if ((m_desc_.createFlags & BufferCreateFlag::CpuAccess) != BufferCreateFlag::None) {
      props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    }
  } else if (m_desc_.type == BufferType::Dynamic) {
    // Dynamic buffers are host visible and coherent
    props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
  }

  // For readback buffers, they need to be host visible and cached
  if ((m_desc_.createFlags & BufferCreateFlag::Readback) != BufferCreateFlag::None) {
    props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
  }

  return props;
}

uint32_t BufferVk::findMemoryType_(uint32_t typeFilter, VkMemoryPropertyFlags properties) const {
  VkPhysicalDeviceMemoryProperties memProperties;
  vkGetPhysicalDeviceMemoryProperties(m_device_->getPhysicalDevice(), &memProperties);

  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
    if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
      return i;
    }
  }

  GlobalLogger::Log(LogLevel::Error, "Failed to find suitable memory type for buffer");
  return 0;
}

}  // namespace rhi
}  // namespace gfx
}  // namespace game_engine
