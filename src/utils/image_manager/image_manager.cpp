#include "utils/image_manager/image_manager.h"

namespace game_engine {

ImageManager::ImageManager(std::shared_ptr<IImageLoader> loader)
    : m_loader_(std::move(loader)) {
}

std::shared_ptr<Image> ImageManager::getImage(
    const std::filesystem::path& filepath) {
  auto it = m_imageCache_.find(filepath);
  if (it != m_imageCache_.end()) {
    return it->second;
  }

  auto image = m_loader_->loadImage(filepath);
  if (image) {
    m_imageCache_[filepath] = image;
    return image;
  }
  return nullptr;
}

}  // namespace game_engine
