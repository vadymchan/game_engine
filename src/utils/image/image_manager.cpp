#include "utils/image/image_manager.h"

#include "utils/image/image_loader_manager.h"
#include "utils/logger/global_logger.h"
#include "utils/service/service_locator.h"

namespace arise {

Image* ImageManager::getImage(const std::filesystem::path& filepath) {
  auto it = m_imageCache_.find(filepath);
  if (it != m_imageCache_.end()) {
    return it->second.get();
  }

  auto imageLoaderManager = ServiceLocator::s_get<ImageLoaderManager>();
  if (!imageLoaderManager) {
    GlobalLogger::Log(LogLevel::Error, "ImageLoaderManager not available in ServiceLocator.");
    return nullptr;
  }

  auto image = imageLoaderManager->loadImage(filepath);
  if (image) {
    Image* imagePtr         = image.get();
    m_imageCache_[filepath] = std::move(image);
    return imagePtr;
  }

  GlobalLogger::Log(LogLevel::Warning, "Failed to load image: " + filepath.string());
  return nullptr;
}

}  // namespace arise