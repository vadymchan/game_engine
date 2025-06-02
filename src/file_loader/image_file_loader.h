#ifndef ARISE_IMAGE_FILE_LOADER_H
#define ARISE_IMAGE_FILE_LOADER_H

#include "resources/image.h"

#include <filesystem>
#include <memory>
#include <string>

namespace arise {

class IImageLoader {
  public:
  virtual ~IImageLoader()                                                           = default;
  virtual std::unique_ptr<Image> loadImage(const std::filesystem::path& filepath)   = 0;
  virtual bool                   supportsFormat(const std::string& extension) const = 0;
};

}  // namespace arise

#endif  // ARISE_IMAGE_FILE_LOADER_H
