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

class BufferManager {
  public:
  BufferManager(gfx::rhi::Device* device);

  ~BufferManager();

  /**
   * @param vertexStride Size of each vertex in bytes
   */
  gfx::rhi::Buffer* createVertexBuffer(const void*        data,
                                       size_t             vertexCount,
                                       size_t             vertexStride,
                                       const std::string& name = "");

  /**
   * @param indexSize Size of each index in bytes (usually 2 or 4)
   */
  gfx::rhi::Buffer* createIndexBuffer(const void*        data,
                                      size_t             indexCount,
                                      size_t             indexSize,
                                      const std::string& name = "");

  /**
   * @param size Size of the buffer in bytes
   */
  gfx::rhi::Buffer* createUniformBuffer(size_t size, const void* data = nullptr, const std::string& name = "");

  /**
   * @param size Size of the buffer in bytes
   */
  gfx::rhi::Buffer* createStorageBuffer(size_t size, const void* data = nullptr, const std::string& name = "");

  gfx::rhi::Buffer* addBuffer(std::unique_ptr<gfx::rhi::Buffer> buffer, const std::string& name);

  gfx::rhi::Buffer* getBuffer(const std::string& name) const;

  bool removeBuffer(const std::string& name);

  bool removeBuffer(gfx::rhi::Buffer* buffer);

  /**
   * @param size Size of the data in bytes
   * @param offset Offset into the buffer in bytes
   */
  bool updateBuffer(const std::string& name, const void* data, size_t size, size_t offset = 0);

  /**
   * @param size Size of the data in bytes
   * @param offset Offset into the buffer in bytes
   */
  bool updateBuffer(gfx::rhi::Buffer* buffer, const void* data, size_t size, size_t offset = 0);

  void release();

  private:
  std::string generateUniqueName_(const std::string& prefix);

  // TODO: not used, consider remove
  std::unordered_map<std::string, std::unique_ptr<gfx::rhi::Buffer>>::iterator findBuffer_(const std::string& name);

  std::unordered_map<std::string, std::unique_ptr<gfx::rhi::Buffer>>::iterator findBuffer_(
      const gfx::rhi::Buffer* buffer);

  private:
  gfx::rhi::Device*                                                  m_device;
  mutable std::mutex                                                 m_mutex;
  std::unordered_map<std::string, std::unique_ptr<gfx::rhi::Buffer>> m_buffers;
  uint32_t m_bufferCounter;  // Counter for generating unique names
};

}  // namespace game_engine

#endif  // GAME_ENGINE_BUFFER_MANAGER_H