#include "utils/buffer/buffer_manager.h"

namespace game_engine {

BufferManager::BufferManager(gfx::rhi::Device* device)
    : m_device(device)
    , m_bufferCounter(0) {
  if (!m_device) {
    GlobalLogger::Log(LogLevel::Error, "Device is null");
  }
}

BufferManager::~BufferManager() {
  release();
}

gfx::rhi::Buffer* BufferManager::createVertexBuffer(const void*        data,
                                                    size_t             vertexCount,
                                                    size_t             vertexStride,
                                                    const std::string& name) {
  if (!m_device) {
    GlobalLogger::Log(LogLevel::Error, "Cannot create vertex buffer, device is null");
    return nullptr;
  }

  if (!data || vertexCount == 0 || vertexStride == 0) {
    GlobalLogger::Log(LogLevel::Error, "Invalid vertex buffer parameters");
    return nullptr;
  }

  // Calculate buffer size
  const size_t bufferSize = vertexCount * vertexStride;

  // Create buffer description
  gfx::rhi::BufferDesc bufferDesc;
  bufferDesc.size        = bufferSize;
  bufferDesc.type        = gfx::rhi::BufferType::Static;
  bufferDesc.createFlags = gfx::rhi::BufferCreateFlag::VertexBuffer;

  // Generate a name if not provided
  std::string bufferName = name.empty() ? generateUniqueName_("VertexBuffer") : name;

  // Create the buffer
  std::lock_guard<std::mutex> lock(m_mutex);

  // Check if a buffer with this name already exists
  if (m_buffers.find(bufferName) != m_buffers.end()) {
    GlobalLogger::Log(LogLevel::Warning, "Buffer with name '" + bufferName + "' already exists, will be replaced");
    m_buffers.erase(bufferName);
  }

  // Create the buffer
  auto buffer = m_device->createBuffer(bufferDesc);
  if (!buffer) {
    GlobalLogger::Log(LogLevel::Error, "Failed to create vertex buffer '" + bufferName + "'");
    return nullptr;
  }

  // Update buffer with vertex data
  m_device->updateBuffer(buffer.get(), data, bufferSize);

  // Store buffer and return raw pointer
  gfx::rhi::Buffer* bufferPtr = buffer.get();
  m_buffers[bufferName]       = std::move(buffer);

  GlobalLogger::Log(LogLevel::Info,
                    "Created vertex buffer '" + bufferName + "' with " + std::to_string(vertexCount) + " vertices");

  return bufferPtr;
}

gfx::rhi::Buffer* BufferManager::createIndexBuffer(const void*        data,
                                                   size_t             indexCount,
                                                   size_t             indexSize,
                                                   const std::string& name) {
  if (!m_device) {
    GlobalLogger::Log(LogLevel::Error, "Cannot create index buffer, device is null");
    return nullptr;
  }

  if (!data || indexCount == 0 || (indexSize != 2 && indexSize != 4)) {
    GlobalLogger::Log(LogLevel::Error, "Invalid index buffer parameters");
    return nullptr;
  }

  // Calculate buffer size
  const size_t bufferSize = indexCount * indexSize;

  // Create buffer description
  gfx::rhi::BufferDesc bufferDesc;
  bufferDesc.size        = bufferSize;
  bufferDesc.type        = gfx::rhi::BufferType::Static;
  bufferDesc.createFlags = gfx::rhi::BufferCreateFlag::IndexBuffer;

  // Generate a name if not provided
  std::string bufferName = name.empty() ? generateUniqueName_("IndexBuffer") : name;

  // Create the buffer
  std::lock_guard<std::mutex> lock(m_mutex);

  // Check if a buffer with this name already exists
  if (m_buffers.find(bufferName) != m_buffers.end()) {
    GlobalLogger::Log(LogLevel::Warning, "Buffer with name '" + bufferName + "' already exists, will be replaced");
    m_buffers.erase(bufferName);
  }

  // Create the buffer
  auto buffer = m_device->createBuffer(bufferDesc);
  if (!buffer) {
    GlobalLogger::Log(LogLevel::Error, "Failed to create index buffer '" + bufferName + "'");
    return nullptr;
  }

  // Update buffer with index data
  m_device->updateBuffer(buffer.get(), data, bufferSize);

  // Store buffer and return raw pointer
  gfx::rhi::Buffer* bufferPtr = buffer.get();
  m_buffers[bufferName]       = std::move(buffer);

  GlobalLogger::Log(LogLevel::Info,
                    "Created index buffer '" + bufferName + "' with " + std::to_string(indexCount) + " indices");

  return bufferPtr;
}

gfx::rhi::Buffer* BufferManager::createUniformBuffer(size_t size, const void* data, const std::string& name) {
  if (!m_device) {
    GlobalLogger::Log(LogLevel::Error, "Cannot create uniform buffer, device is null");
    return nullptr;
  }

  if (size == 0) {
    GlobalLogger::Log(LogLevel::Error, "Invalid uniform buffer size");
    return nullptr;
  }

  // Create buffer description
  gfx::rhi::BufferDesc bufferDesc;
  bufferDesc.size        = size;
  bufferDesc.type        = gfx::rhi::BufferType::Dynamic;          // Uniform buffers are typically updated frequently
  bufferDesc.createFlags = gfx::rhi::BufferCreateFlag::CpuAccess;  // For easy updates

  // Generate a name if not provided
  std::string bufferName = name.empty() ? generateUniqueName_("UniformBuffer") : name;

  // Create the buffer
  std::lock_guard<std::mutex> lock(m_mutex);

  // Check if a buffer with this name already exists
  if (m_buffers.find(bufferName) != m_buffers.end()) {
    GlobalLogger::Log(LogLevel::Warning, "Buffer with name '" + bufferName + "' already exists, will be replaced");
    m_buffers.erase(bufferName);
  }

  // Create the buffer
  auto buffer = m_device->createBuffer(bufferDesc);
  if (!buffer) {
    GlobalLogger::Log(LogLevel::Error, "Failed to create uniform buffer '" + bufferName + "'");
    return nullptr;
  }

  // Update buffer with initial data if provided
  if (data) {
    m_device->updateBuffer(buffer.get(), data, size);
  }

  // Store buffer and return raw pointer
  gfx::rhi::Buffer* bufferPtr = buffer.get();
  m_buffers[bufferName]       = std::move(buffer);

  GlobalLogger::Log(LogLevel::Info,
                    "Created uniform buffer '" + bufferName + "' with size " + std::to_string(size) + " bytes");

  return bufferPtr;
}

gfx::rhi::Buffer* BufferManager::createStorageBuffer(size_t size, const void* data, const std::string& name) {
  if (!m_device) {
    GlobalLogger::Log(LogLevel::Error, "Cannot create storage buffer, device is null");
    return nullptr;
  }

  if (size == 0) {
    GlobalLogger::Log(LogLevel::Error, "Invalid storage buffer size");
    return nullptr;
  }

  // Create buffer description
  gfx::rhi::BufferDesc bufferDesc;
  bufferDesc.size = size;
  bufferDesc.type = gfx::rhi::BufferType::Dynamic;
  // Storage buffers need UAV access
  bufferDesc.createFlags
      = static_cast<gfx::rhi::BufferCreateFlag>(static_cast<uint32_t>(gfx::rhi::BufferCreateFlag::CpuAccess)
                                                | static_cast<uint32_t>(gfx::rhi::BufferCreateFlag::Uav));

  // Generate a name if not provided
  std::string bufferName = name.empty() ? generateUniqueName_("StorageBuffer") : name;

  // Create the buffer
  std::lock_guard<std::mutex> lock(m_mutex);

  // Check if a buffer with this name already exists
  if (m_buffers.find(bufferName) != m_buffers.end()) {
    GlobalLogger::Log(LogLevel::Warning, "Buffer with name '" + bufferName + "' already exists, will be replaced");
    m_buffers.erase(bufferName);
  }

  // Create the buffer
  auto buffer = m_device->createBuffer(bufferDesc);
  if (!buffer) {
    GlobalLogger::Log(LogLevel::Error, "Failed to create storage buffer '" + bufferName + "'");
    return nullptr;
  }

  // Update buffer with initial data if provided
  if (data) {
    m_device->updateBuffer(buffer.get(), data, size);
  }

  // Store buffer and return raw pointer
  gfx::rhi::Buffer* bufferPtr = buffer.get();
  m_buffers[bufferName]       = std::move(buffer);

  GlobalLogger::Log(LogLevel::Info,
                    "Created storage buffer '" + bufferName + "' with size " + std::to_string(size) + " bytes");

  return bufferPtr;
}

gfx::rhi::Buffer* BufferManager::addBuffer(std::unique_ptr<gfx::rhi::Buffer> buffer, const std::string& name) {
  if (!buffer) {
    GlobalLogger::Log(LogLevel::Error, "Cannot add null buffer");
    return nullptr;
  }

  std::string bufferName = name.empty() ? generateUniqueName_("Buffer") : name;

  std::lock_guard<std::mutex> lock(m_mutex);

  // Check if a buffer with this name already exists
  if (m_buffers.find(bufferName) != m_buffers.end()) {
    GlobalLogger::Log(LogLevel::Warning, "Buffer with name '" + bufferName + "' already exists, will be replaced");
    m_buffers.erase(bufferName);
  }

  // Store buffer and return raw pointer
  gfx::rhi::Buffer* bufferPtr = buffer.get();
  m_buffers[bufferName]       = std::move(buffer);

  GlobalLogger::Log(LogLevel::Info, "Added external buffer '" + bufferName + "'");

  return bufferPtr;
}

gfx::rhi::Buffer* BufferManager::getBuffer(const std::string& name) const {
  std::lock_guard<std::mutex> lock(m_mutex);

  auto it = m_buffers.find(name);
  if (it != m_buffers.end()) {
    return it->second.get();
  }

  return nullptr;
}

bool BufferManager::removeBuffer(const std::string& name) {
  std::lock_guard<std::mutex> lock(m_mutex);

  auto it = m_buffers.find(name);
  if (it != m_buffers.end()) {
    GlobalLogger::Log(LogLevel::Info, "Removing buffer '" + name + "'");
    m_buffers.erase(it);
    return true;
  }

  GlobalLogger::Log(LogLevel::Warning, "Attempted to remove non-existent buffer '" + name + "'");
  return false;
}

bool BufferManager::updateBuffer(const std::string& name, const void* data, size_t size, size_t offset) {
  if (!m_device || !data || size == 0) {
    GlobalLogger::Log(LogLevel::Error, "Invalid update buffer parameters");
    return false;
  }

  std::lock_guard<std::mutex> lock(m_mutex);

  auto it = m_buffers.find(name);
  if (it == m_buffers.end()) {
    GlobalLogger::Log(LogLevel::Error, "Cannot update buffer '" + name + "', not found");
    return false;
  }

  m_device->updateBuffer(it->second.get(), data, size, offset);
  return true;
}

bool BufferManager::updateBuffer(gfx::rhi::Buffer* buffer, const void* data, size_t size, size_t offset) {
  if (!m_device || !buffer || !data || size == 0) {
    GlobalLogger::Log(LogLevel::Error, "Invalid update buffer parameters");
    return false;
  }

  // Find the buffer in our map to ensure we own it
  std::lock_guard<std::mutex> lock(m_mutex);

  auto it = findBuffer_(buffer);
  if (it == m_buffers.end()) {
    GlobalLogger::Log(LogLevel::Warning, "Updating buffer not managed by this BufferManager");
  }

  m_device->updateBuffer(buffer, data, size, offset);
  return true;
}

void BufferManager::release() {
  std::lock_guard<std::mutex> lock(m_mutex);

  GlobalLogger::Log(LogLevel::Info, "Releasing " + std::to_string(m_buffers.size()) + " buffers");

  m_buffers.clear();
}

std::string BufferManager::generateUniqueName_(const std::string& prefix) {
  return prefix + "_" + std::to_string(m_bufferCounter++);
}

std::unordered_map<std::string, std::unique_ptr<gfx::rhi::Buffer>>::iterator BufferManager::findBuffer_(
    const std::string& name) {
  return m_buffers.find(name);
}

std::unordered_map<std::string, std::unique_ptr<gfx::rhi::Buffer>>::iterator BufferManager::findBuffer_(
    const gfx::rhi::Buffer* buffer) {
  for (auto it = m_buffers.begin(); it != m_buffers.end(); ++it) {
    if (it->second.get() == buffer) {
      return it;
    }
  }
  return m_buffers.end();
}

}  // namespace game_engine