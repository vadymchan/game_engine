#include "utils/image/image_manager.h"

#include "utils/image/image_loader_manager.h"
#include "utils/service/service_locator.h"

namespace game_engine {

std::shared_ptr<Image> ImageManager::getImage(
    const std::filesystem::path& filepath) {
  auto it = m_imageCache_.find(filepath);
  if (it != m_imageCache_.end()) {
    return it->second;
  }

  auto imageLoaderManager = ServiceLocator::s_get<ImageLoaderManager>();
  if (!imageLoaderManager) {
    // TODO: add logging
    std::cerr << "ImageLoaderManager not available in ServiceLocator."
              << std::endl;
    return nullptr;
  }

  auto image = imageLoaderManager->loadImage(filepath);
  if (image) {
    m_imageCache_[filepath] = image;
    return image;
  }
  return nullptr;
}

}  // namespace game_engine
