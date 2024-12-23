#ifndef GAME_ENGINE_IMAGE_MANAGER_H
#define GAME_ENGINE_IMAGE_MANAGER_H

#include "file_loader/image_file_loader.h"

#include <filesystem>

namespace game_engine {

class ImageManager {
  public:
  ImageManager() = default;

  std::shared_ptr<Image> getImage(const std::filesystem::path& filepath);

  private:
  std::unordered_map<std::filesystem::path, std::shared_ptr<Image>>
      m_imageCache_;
};
}  // namespace game_engine

#endif  // GAME_ENGINE_IMAGE_MANAGER_H
