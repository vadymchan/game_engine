#include "utils/image/image_loader_manager.h"

#include "utils/logger/global_logger.h"

namespace game_engine {

void ImageLoaderManager::registerLoader(EImageType                    imageType,
                                        std::shared_ptr<IImageLoader> loader) {
  std::lock_guard<std::mutex> lock(mutex_);
  loaderMap_[imageType] = std::move(loader);
}

std::shared_ptr<Image> ImageLoaderManager::loadImage(
    const std::filesystem::path& filepath) {
  std::string extension = filepath.extension().string();
  std::transform(
      extension.begin(), extension.end(), extension.begin(), ::tolower);
  EImageType imageType = getImageTypeFromExtension(extension);

  if (imageType == EImageType::UNKNOWN) {
    GlobalLogger::Log(LogLevel::Error,
                      "Unknown image type for extension: " + extension);
    return nullptr;
  }

  std::shared_ptr<IImageLoader> loader = nullptr;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    auto                        it = loaderMap_.find(imageType);
    if (it != loaderMap_.end()) {
      loader = it->second;
    }
  }

  if (loader && loader->supportsFormat(extension)) {
    return loader->loadImage(filepath);
  }
  GlobalLogger::Log(LogLevel::Error,
                    "No suitable loader found for image type: " + extension);
  return nullptr;
}

}  // namespace game_engine
