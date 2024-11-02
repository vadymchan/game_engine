#include "utils/third_party/stb_util.h"

#include <unordered_set>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace game_engine {

bool STBImageLoader::isSupportedImageFormat_(
    const std::filesystem::path& filepath) const {
  static const std::unordered_set<std::string_view> supportedExtensions
      = {".jpeg",
         ".jpg",
         ".png",
         ".bmp",
         ".tga",
         ".gif",
         ".hdr",
         ".pic",
         ".ppm",
         ".pgm"};

  std::string extension = filepath.extension().string();
  std::transform(
      extension.begin(), extension.end(), extension.begin(), ::tolower);

  return supportedExtensions.find(extension) != supportedExtensions.end();
}

std::shared_ptr<Image> STBImageLoader::loadImage(
    const std::filesystem::path& filepath) {
  if (!isSupportedImageFormat_(filepath)) {
    // TODO: use logger
    std::cerr << "Unsupported image format: " << filepath.extension().string()
              << std::endl;
    return nullptr;
  }

  if (stbi_is_hdr(filepath.string().c_str())) {
    return loadImageData_(filepath, &loadHdr_, stbi_image_free, 32, true);
  } else if (stbi_is_16_bit(filepath.string().c_str())) {
    return loadImageData_(filepath, &load16Bit_, stbi_image_free, 16, false);
  } else {
    return loadImageData_(filepath, &load8Bit_, stbi_image_free, 8, false);
  }
}

void* STBImageLoader::loadHdr_(const std::filesystem::path& filepath,
                               int32_t*                     x,
                               int32_t*                     y,
                               int32_t*                     channelsInFile,
                               int32_t                      desiredChannels) {
  return static_cast<void*>(stbi_loadf(
      filepath.string().c_str(), x, y, channelsInFile, desiredChannels));
}

void* STBImageLoader::load16Bit_(const std::filesystem::path& filepath,
                                 int32_t*                     x,
                                 int32_t*                     y,
                                 int32_t*                     channelsInFile,
                                 int32_t                      desiredChannels) {
  return static_cast<void*>(stbi_load_16(
      filepath.string().c_str(), x, y, channelsInFile, desiredChannels));
}

void* STBImageLoader::load8Bit_(const std::filesystem::path& filepath,
                                int32_t*                     x,
                                int32_t*                     y,
                                int32_t*                     channelsInFile,
                                int32_t                      desiredChannels) {
  return static_cast<void*>(stbi_load(
      filepath.string().c_str(), x, y, channelsInFile, desiredChannels));
}

std::shared_ptr<Image> STBImageLoader::loadImageData_(
    const std::filesystem::path& filepath,
    LoaderFunc                   loader,
    FreeFunc                     freeFunc,
    int32_t                      bitsPerChannel,
    bool                         isHdr) {
  int32_t width, height, channels;
  // TODO: make it parameterized
  int32_t desiredChannels = 0;

  auto* data = loader(filepath, &width, &height, &channels, desiredChannels);
  if (!data) {
    // TODO: use logger
    std::cerr << "Failed to load image: " << filepath << std::endl;
    return nullptr;
  }

  const size_t imageSize = static_cast<size_t>(width) * height * channels
                         * (bitsPerChannel / CHAR_BIT);

  std::vector<std::byte> imageData(
      reinterpret_cast<std::byte*>(data),
      reinterpret_cast<std::byte*>(data) + imageSize);

  freeFunc(data);
  return std::make_shared<Image>(Image{
    width, height, channels, bitsPerChannel, isHdr, std::move(imageData)});
}

}  // namespace game_engine
