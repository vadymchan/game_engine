#ifndef GAME_ENGINE_TEXTURE_MANAGER_H
#define GAME_ENGINE_TEXTURE_MANAGER_H

#include "gfx/rhi/interface/device.h"
#include "gfx/rhi/interface/texture.h"
#include "resources/image.h"
#include "utils/logger/global_logger.h"

#include <filesystem>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

namespace game_engine {

/**
 * @brief Manages the lifecycle of GPU texture resources.
 *
 * This class is responsible for creating, storing, and managing GPU textures.
 * It provides functions to create textures from various sources (images, files, etc.)
 * and manages their lifecycle.
 */
class TextureManager {
  public:
  /**
   * @brief Constructor
   * @param device The RHI device used to create resources
   */
  TextureManager(gfx::rhi::Device* device);

  /**
   * @brief Destructor, releases all managed textures
   */
  ~TextureManager();

  /**
   * @brief Create a texture from an image
   *
   * @param image Source image data
   * @param name Optional name for the texture
   * @return Raw pointer to the created texture (owned by this manager)
   */
  gfx::rhi::Texture* createTexture(Image* image, const std::string& name = "");

  /**
   * @brief Create a texture from a file
   *
   * @param filepath Path to the image file
   * @param name Optional name for the texture (defaults to filename)
   * @return Raw pointer to the created texture (owned by this manager)
   */
  gfx::rhi::Texture* createTextureFromFile(const std::filesystem::path& filepath, const std::string& name = "");

  /**
   * @brief Create a render target texture
   *
   * @param width Width of the texture
   * @param height Height of the texture
   * @param format Texture format
   * @param name Optional name for the texture
   * @return Raw pointer to the created texture (owned by this manager)
   */
  gfx::rhi::Texture* createRenderTarget(uint32_t                width,
                                        uint32_t                height,
                                        gfx::rhi::TextureFormat format = gfx::rhi::TextureFormat::Rgba8,
                                        const std::string&      name   = "");

  /**
   * @brief Create a depth-stencil texture
   *
   * @param width Width of the texture
   * @param height Height of the texture
   * @param format Texture format (must be a depth format)
   * @param name Optional name for the texture
   * @return Raw pointer to the created texture (owned by this manager)
   */
  gfx::rhi::Texture* createDepthStencil(uint32_t                width,
                                        uint32_t                height,
                                        gfx::rhi::TextureFormat format = gfx::rhi::TextureFormat::D24S8,
                                        const std::string&      name   = "");

  /**
   * @brief Add an existing texture to the manager
   *
   * @param texture Texture to add (ownership is transferred to this manager)
   * @param name Name for the texture
   * @return Raw pointer to the texture
   */
  gfx::rhi::Texture* addTexture(std::unique_ptr<gfx::rhi::Texture> texture, const std::string& name);

  /**
   * @brief Get a texture by name
   *
   * @param name Name of the texture
   * @return Raw pointer to the texture (owned by this manager), nullptr if not found
   */
  gfx::rhi::Texture* getTexture(const std::string& name) const;

  /**
   * @brief Remove a texture from the manager
   *
   * @param name Name of the texture to remove
   * @return true if texture was found and removed, false otherwise
   */
  bool removeTexture(const std::string& name);

  /**
   * @brief Check if a texture exists in the manager
   *
   * @param name Name of the texture
   * @return true if texture exists, false otherwise
   */
  bool hasTexture(const std::string& name) const;

  /**
   * @brief Get the number of textures managed by this manager
   *
   * @return Number of textures
   */
  size_t getTextureCount() const;

  /**
   * @brief Release all textures managed by this manager
   */
  void release();

  private:
  /**
   * @brief Generate a unique name for a texture
   *
   * @param prefix Prefix to use in name
   * @return Unique texture name
   */
  std::string generateUniqueName_(const std::string& prefix);

  /**
   * @brief Find a texture by name
   *
   * @param name Name of the texture
   * @return Iterator to the texture in the map, m_textures_.end() if not found
   */
  std::unordered_map<std::string, std::unique_ptr<gfx::rhi::Texture>>::iterator findTexture_(const std::string& name);

  /**
   * @brief Find a texture by pointer
   *
   * @param texture Pointer to the texture
   * @return Iterator to the texture in the map, m_textures_.end() if not found
   */
  std::unordered_map<std::string, std::unique_ptr<gfx::rhi::Texture>>::iterator findTexture_(
      const gfx::rhi::Texture* texture);

  private:
  gfx::rhi::Device*  m_device;  // RHI device, not owned by this manager
  mutable std::mutex m_mutex;   // Mutex for thread safety
  std::unordered_map<std::string, std::unique_ptr<gfx::rhi::Texture>> m_textures;  // Owned textures
  uint32_t m_textureCounter;  // Counter for generating unique names
};

}  // namespace game_engine

#endif  // GAME_ENGINE_TEXTURE_MANAGER_H