#include "utils/texture/texture_manager.h"

#include "utils/image/image_manager.h"
#include "utils/service/service_locator.h"

namespace game_engine {

TextureManager::TextureManager(gfx::rhi::Device* device)
    : m_device(device)
    , m_textureCounter(0) {
  if (!m_device) {
    GlobalLogger::Log(LogLevel::Error, "Device is null");
  }
}

TextureManager::~TextureManager() {
  release();
}

gfx::rhi::Texture* TextureManager::createTexture(Image* image, const std::string& name) {
  if (!m_device) {
    GlobalLogger::Log(LogLevel::Error, "Cannot create texture, device is null");
    return nullptr;
  }

  if (!image) {
    GlobalLogger::Log(LogLevel::Error, "Cannot create texture, image is null");
    return nullptr;
  }

  // Create texture descriptor
  gfx::rhi::TextureDesc desc;
  desc.width       = static_cast<uint32_t>(image->width);
  desc.height      = static_cast<uint32_t>(image->height);
  desc.depth       = static_cast<uint32_t>(image->depth);
  desc.format      = image->format;
  desc.type        = image->dimension;
  desc.mipLevels   = static_cast<uint32_t>(image->mipLevels);
  desc.arraySize   = static_cast<uint32_t>(image->arraySize);
  desc.createFlags = gfx::rhi::TextureCreateFlag::TransferDst;

  // Generate a name if not provided
  std::string textureName = name.empty() ? generateUniqueName_("Texture") : name;

  // Create the texture
  std::lock_guard<std::mutex> lock(m_mutex);

  // Check if a texture with this name already exists
  if (m_textures.find(textureName) != m_textures.end()) {
    GlobalLogger::Log(LogLevel::Warning,
                      "Texture with name '" + textureName + "' already exists, will be replaced");
    m_textures.erase(textureName);
  }

  // Create the texture
  auto texture = m_device->createTexture(desc);
  if (!texture) {
    GlobalLogger::Log(LogLevel::Error, "Failed to create texture '" + textureName + "'");
    return nullptr;
  }

  // Update the texture with image data
  if (!image->pixels.empty()) {
    m_device->updateTexture(texture.get(), image->pixels.data(), image->pixels.size());
  }

  // Store texture and return raw pointer
  gfx::rhi::Texture* texturePtr = texture.get();
  m_textures[textureName]       = std::move(texture);

  GlobalLogger::Log(LogLevel::Info,
                    "Created texture '" + textureName + "' with dimensions "
                        + std::to_string(desc.width) + "x" + std::to_string(desc.height));

  return texturePtr;
}

gfx::rhi::Texture* TextureManager::createTextureFromFile(const std::filesystem::path& filepath,
                                                         const std::string&           name) {
  // Try to get ImageManager from ServiceLocator
  auto imageManager = ServiceLocator::s_get<ImageManager>();
  if (!imageManager) {
    GlobalLogger::Log(LogLevel::Error, "Cannot create texture from file, ImageManager not available");
    return nullptr;
  }

  // Load the image
  Image* image = imageManager->getImage(filepath);
  if (!image) {
    GlobalLogger::Log(LogLevel::Error, "Failed to load image from file: " + filepath.string());
    return nullptr;
  }

  // Use filename as texture name if not provided
  std::string textureName = name.empty() ? filepath.filename().string() : name;

  // Create texture from image
  return createTexture(image, textureName);
}

gfx::rhi::Texture* TextureManager::createRenderTarget(uint32_t                width,
                                                      uint32_t                height,
                                                      gfx::rhi::TextureFormat format,
                                                      const std::string&      name) {
  if (!m_device) {
    GlobalLogger::Log(LogLevel::Error, "Cannot create render target, device is null");
    return nullptr;
  }

  if (width == 0 || height == 0) {
    GlobalLogger::Log(LogLevel::Error, "Invalid render target dimensions");
    return nullptr;
  }

  // Create texture descriptor
  gfx::rhi::TextureDesc desc;
  desc.width     = width;
  desc.height    = height;
  desc.depth     = 1;
  desc.format    = format;
  desc.type      = gfx::rhi::TextureType::Texture2D;
  desc.mipLevels = 1;
  desc.arraySize = 1;
  desc.createFlags = gfx::rhi::TextureCreateFlag::Rtv;

  // Generate a name if not provided
  std::string textureName = name.empty() ? generateUniqueName_("RenderTarget") : name;

  // Create the texture
  std::lock_guard<std::mutex> lock(m_mutex);

  // Check if a texture with this name already exists
  if (m_textures.find(textureName) != m_textures.end()) {
    GlobalLogger::Log(LogLevel::Warning,
                      "Texture with name '" + textureName + "' already exists, will be replaced");
    m_textures.erase(textureName);
  }

  // Create the texture
  auto texture = m_device->createTexture(desc);
  if (!texture) {
    GlobalLogger::Log(LogLevel::Error, "Failed to create render target '" + textureName + "'");
    return nullptr;
  }

  // Store texture and return raw pointer
  gfx::rhi::Texture* texturePtr = texture.get();
  m_textures[textureName]       = std::move(texture);

  GlobalLogger::Log(LogLevel::Info,
                    "Created render target '" + textureName + "' with dimensions "
                        + std::to_string(width) + "x" + std::to_string(height));

  return texturePtr;
}

gfx::rhi::Texture* TextureManager::createDepthStencil(uint32_t                width,
                                                      uint32_t                height,
                                                      gfx::rhi::TextureFormat format,
                                                      const std::string&      name) {
  if (!m_device) {
    GlobalLogger::Log(LogLevel::Error, "Cannot create depth stencil, device is null");
    return nullptr;
  }

  if (width == 0 || height == 0) {
    GlobalLogger::Log(LogLevel::Error, "Invalid depth stencil dimensions");
    return nullptr;
  }

  // Validate that the format is a depth format
  if (!gfx::rhi::g_isDepthFormat(format)) {
    GlobalLogger::Log(LogLevel::Error, "Invalid format for depth stencil, must be a depth format");
    return nullptr;
  }

  // Create texture descriptor
  gfx::rhi::TextureDesc desc;
  desc.width     = width;
  desc.height    = height;
  desc.depth     = 1;
  desc.format    = format;
  desc.type      = gfx::rhi::TextureType::Texture2D;
  desc.mipLevels = 1;
  desc.arraySize = 1;
  desc.createFlags = gfx::rhi::TextureCreateFlag::Dsv;

  // Generate a name if not provided
  std::string textureName = name.empty() ? generateUniqueName_("DepthStencil") : name;

  // Create the texture
  std::lock_guard<std::mutex> lock(m_mutex);

  // Check if a texture with this name already exists
  if (m_textures.find(textureName) != m_textures.end()) {
    GlobalLogger::Log(LogLevel::Warning,
                      "Texture with name '" + textureName + "' already exists, will be replaced");
    m_textures.erase(textureName);
  }

  // Create the texture
  auto texture = m_device->createTexture(desc);
  if (!texture) {
    GlobalLogger::Log(LogLevel::Error, "Failed to create depth stencil '" + textureName + "'");
    return nullptr;
  }

  // Store texture and return raw pointer
  gfx::rhi::Texture* texturePtr = texture.get();
  m_textures[textureName]       = std::move(texture);

  GlobalLogger::Log(LogLevel::Info,
                    "Created depth stencil '" + textureName + "' with dimensions "
                        + std::to_string(width) + "x" + std::to_string(height));

  return texturePtr;
}

gfx::rhi::Texture* TextureManager::addTexture(std::unique_ptr<gfx::rhi::Texture> texture, const std::string& name) {
  if (!texture) {
    GlobalLogger::Log(LogLevel::Error, "Cannot add null texture");
    return nullptr;
  }

  std::string textureName = name.empty() ? generateUniqueName_("Texture") : name;

  std::lock_guard<std::mutex> lock(m_mutex);

  // Check if a texture with this name already exists
  if (m_textures.find(textureName) != m_textures.end()) {
    GlobalLogger::Log(LogLevel::Warning,
                      "Texture with name '" + textureName + "' already exists, will be replaced");
    m_textures.erase(textureName);
  }

  // Store texture and return raw pointer
  gfx::rhi::Texture* texturePtr = texture.get();
  m_textures[textureName]       = std::move(texture);

  GlobalLogger::Log(LogLevel::Info, "Added external texture '" + textureName + "'");

  return texturePtr;
}

gfx::rhi::Texture* TextureManager::getTexture(const std::string& name) const {
  std::lock_guard<std::mutex> lock(m_mutex);

  auto it = m_textures.find(name);
  if (it != m_textures.end()) {
    return it->second.get();
  }

  return nullptr;
}

bool TextureManager::removeTexture(const std::string& name) {
  std::lock_guard<std::mutex> lock(m_mutex);

  auto it = m_textures.find(name);
  if (it != m_textures.end()) {
    GlobalLogger::Log(LogLevel::Info, "Removing texture '" + name + "'");
    m_textures.erase(it);
    return true;
  }

  GlobalLogger::Log(LogLevel::Warning, "Attempted to remove non-existent texture '" + name + "'");
  return false;
}

bool TextureManager::hasTexture(const std::string& name) const {
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_textures.find(name) != m_textures.end();
}

size_t TextureManager::getTextureCount() const {
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_textures.size();
}

void TextureManager::release() {
  std::lock_guard<std::mutex> lock(m_mutex);

  GlobalLogger::Log(LogLevel::Info, "Releasing " + std::to_string(m_textures.size()) + " textures");

  m_textures.clear();
}

std::string TextureManager::generateUniqueName_(const std::string& prefix) {
  return prefix + "_" + std::to_string(m_textureCounter++);
}

std::unordered_map<std::string, std::unique_ptr<gfx::rhi::Texture>>::iterator TextureManager::findTexture_(
    const std::string& name) {
  return m_textures.find(name);
}

std::unordered_map<std::string, std::unique_ptr<gfx::rhi::Texture>>::iterator TextureManager::findTexture_(
    const gfx::rhi::Texture* texture) {
  for (auto it = m_textures.begin(); it != m_textures.end(); ++it) {
    if (it->second.get() == texture) {
      return it;
    }
  }
  return m_textures.end();
}

}  // namespace game_engine