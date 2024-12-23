#ifndef GAME_ENGINE_IMAGE_LOADER_MANAGER_H
#define GAME_ENGINE_IMAGE_LOADER_MANAGER_H

#include "file_loader/image_file_loader.h"

#include <filesystem>
#include <memory>
#include <mutex>
#include <unordered_map>

namespace game_engine {

class ImageLoaderManager {
  public:
  ImageLoaderManager() = default;

  void registerLoader(EImageType                    imageType,
                      std::shared_ptr<IImageLoader> loader);

  std::shared_ptr<Image> loadImage(const std::filesystem::path& filepath);

  private:
  std::unordered_map<EImageType, std::shared_ptr<IImageLoader>> loaderMap_;
  std::mutex                                                    mutex_;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_IMAGE_LOADER_MANAGER_H
