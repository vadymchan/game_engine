#ifndef ARISE_IMAGE_MANAGER_H
#define ARISE_IMAGE_MANAGER_H

#include "file_loader/image_file_loader.h"

#include <filesystem>
#include <memory>
#include <unordered_map>

namespace arise {

class ImageManager {
  public:
  ImageManager() = default;

  Image* getImage(const std::filesystem::path& filepath);

  private:
  std::unordered_map<std::filesystem::path, std::unique_ptr<Image>> m_imageCache_;
};

}  // namespace arise

#endif  // ARISE_IMAGE_MANAGER_H