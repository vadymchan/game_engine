// TODO: rename this file to stb_utils.h

#ifndef GAME_ENGINE_STB_UTIL_H
#define GAME_ENGINE_STB_UTIL_H

#include "file_loader/image_file_loader.h"

#include <stb_image.h>

#include <filesystem>
#include <functional>
#include <unordered_set>

namespace game_engine {

class STBImageLoader : public IImageLoader {
  public:
  std::unique_ptr<Image> loadImage(const std::filesystem::path& filepath) override;

  bool supportsFormat(const std::string& extension) const override;

  private:
  using LoaderFunc = std::function<void*(const std::filesystem::path&, int32_t*, int32_t*, int32_t*, int32_t)>;
  using FreeFunc   = std::function<void(void*)>;

  static void* loadHdr_(const std::filesystem::path& filepath,
                        int32_t*                     x,
                        int32_t*                     y,
                        int32_t*                     channelsInFile,
                        int32_t                      desiredChannels = 0);

  static void* load16Bit_(const std::filesystem::path& filepath,
                          int32_t*                     x,
                          int32_t*                     y,
                          int32_t*                     channelsInFile,
                          int32_t                      desiredChannels = 0);

  static void* load8Bit_(const std::filesystem::path& filepath,
                         int32_t*                     x,
                         int32_t*                     y,
                         int32_t*                     channelsInFile,
                         int32_t                      desiredChannels = 0);

  std::unique_ptr<Image> loadImageData_(
      const std::filesystem::path& filepath, LoaderFunc loader, FreeFunc freeFunc, int32_t bitsPerChannel, bool isHdr);

  gfx::rhi::TextureFormat determineFormat_(int32_t channels, int32_t bitsPerChannel, bool isHdr);

  void generateMipmaps_(std::unique_ptr<Image>& image, int32_t channels, int32_t bitsPerChannel);

  static const std::unordered_set<std::string> supportedExtensions_;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_STB_UTIL_H