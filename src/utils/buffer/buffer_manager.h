#ifndef GAME_ENGINE_BUFFER_MANAGER_H
#define GAME_ENGINE_BUFFER_MANAGER_H

#include "gfx/rhi/interface/buffer.h"

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

namespace game_engine::gfx::rhi {
class Device;
}  // namespace game_engine::gfx::rhi

namespace game_engine {

/**
 * @brief Manages the lifecycle of GPU buffer resources.
 */
class BufferManager {
  public:
  BufferManager(gfx::rhi::Device* device);

  ~BufferManager();

  /**
   * @brief Create a vertex buffer from vertex data
   *
   * @param data Pointer to vertex data
   * @param vertexCount Number of vertices
   * @param vertexStride Size of each vertex in bytes
   * @param name Optional name for the buffer
   * @return Raw pointer to the created buffer (owned by this manager)
   */
  gfx::rhi::Buffer* createVertexBuffer(const void*        data,
                                       size_t             vertexCount,
                                       size_t             vertexStride,
                                       const std::string& name = "");

  /**
   * @brief Create an index buffer from index data
   *
   * @param data Pointer to index data
   * @param indexCount Number of indices
   * @param indexSize Size of each index in bytes (usually 2 or 4)
   * @param name Optional name for the buffer
   * @return Raw pointer to the created buffer (owned by this manager)
   */
  gfx::rhi::Buffer* createIndexBuffer(const void*        data,
                                      size_t             indexCount,
                                      size_t             indexSize,
                                      const std::string& name = "");

  /**
   * @brief Create a uniform buffer
   *
   * @param size Size of the buffer in bytes
   * @param data Optional initial data
   * @param name Optional name for the buffer
   * @return Raw pointer to the created buffer (owned by this manager)
   */
  gfx::rhi::Buffer* createUniformBuffer(size_t size, const void* data = nullptr, const std::string& name = "");

  /**
   * @brief Create a storage buffer for compute/read-write access
   *
   * @param size Size of the buffer in bytes
   * @param data Optional initial data
   * @param name Optional name for the buffer
   * @return Raw pointer to the created buffer (owned by this manager)
   */
  gfx::rhi::Buffer* createStorageBuffer(size_t size, const void* data = nullptr, const std::string& name = "");

  /**
   * @brief Add an existing buffer to the manager
   *
   * @param buffer Buffer to add (ownership is transferred to this manager)
   * @param name Name for the buffer
   * @return Raw pointer to the buffer
   */
  gfx::rhi::Buffer* addBuffer(std::unique_ptr<gfx::rhi::Buffer> buffer, const std::string& name);

  /**
   * @brief Get a buffer by name
   *
   * @param name Name of the buffer
   * @return Raw pointer to the buffer (owned by this manager), nullptr if not found
   */
  gfx::rhi::Buffer* getBuffer(const std::string& name) const;

  /**
   * @brief Remove a buffer from the manager
   *
   * @param name Name of the buffer to remove
   * @return true if buffer was found and removed, false otherwise
   */
  bool removeBuffer(const std::string& name);

  /**
   * @brief Update a buffer's data
   *
   * @param name Name of the buffer
   * @param data New data
   * @param size Size of the data in bytes
   * @param offset Offset into the buffer in bytes
   * @return true if update was successful, false otherwise
   */
  bool updateBuffer(const std::string& name, const void* data, size_t size, size_t offset = 0);

  /**
   * @brief Update a buffer's data using the buffer pointer
   *
   * @param buffer Buffer to update
   * @param data New data
   * @param size Size of the data in bytes
   * @param offset Offset into the buffer in bytes
   * @return true if update was successful, false otherwise
   */
  bool updateBuffer(gfx::rhi::Buffer* buffer, const void* data, size_t size, size_t offset = 0);

  /**
   * @brief Release all buffers managed by this manager
   */
  void release();

  private:
  /**
   * @brief Generate a unique name for a buffer
   *
   * @param prefix Prefix to use in name
   * @return Unique buffer name
   */
  std::string generateUniqueName_(const std::string& prefix);

  /**
   * @brief Find a buffer by name
   *
   * @param name Name of the buffer
   * @return Iterator to the buffer in the map, m_buffers_.end() if not found
   */
  std::unordered_map<std::string, std::unique_ptr<gfx::rhi::Buffer>>::iterator findBuffer_(const std::string& name);

  /**
   * @brief Find a buffer by pointer
   *
   * @param buffer Pointer to the buffer
   * @return Iterator to the buffer in the map, m_buffers_.end() if not found
   */
  std::unordered_map<std::string, std::unique_ptr<gfx::rhi::Buffer>>::iterator findBuffer_(
      const gfx::rhi::Buffer* buffer);

  private:
  gfx::rhi::Device*                                                  m_device;  // RHI device, not owned by this manager
  mutable std::mutex                                                 m_mutex;   // Mutex for thread safety
  std::unordered_map<std::string, std::unique_ptr<gfx::rhi::Buffer>> m_buffers;  // Owned buffers
  uint32_t m_bufferCounter;                                                      // Counter for generating unique names
};

}  // namespace game_engine

#endif  // GAME_ENGINE_BUFFER_MANAGER_H