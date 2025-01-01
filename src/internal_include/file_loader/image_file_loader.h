#ifndef GAME_ENGINE_IMAGE_FILE_LOADER_H
#define GAME_ENGINE_IMAGE_FILE_LOADER_H

#include "gfx/rhi/name.h"
#include "gfx/rhi/rhi_type.h"
#include "resources/image.h"

// TODO: remove
#include <vulkan/vulkan.h>

#include <filesystem>

namespace game_engine {

class IImageLoader {
  public:
  virtual ~IImageLoader() = default;
  virtual std::shared_ptr<Image> loadImage(
      const std::filesystem::path& filepath)
      = 0;
  virtual bool supportsFormat(const std::string& extension) const = 0;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_IMAGE_FILE_LOADER_H
