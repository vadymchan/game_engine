#include "utils/third_party/stb_util.h"

#include "utils/logger/global_logger.h"

#include <unordered_set>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace game_engine {

const std::unordered_set<std::string> STBImageLoader::supportedExtensions_
    = {".jpeg", ".jpg", ".png", ".bmp", ".tga", ".gif", ".hdr", ".pic", ".ppm", ".pgm"};

bool STBImageLoader::supportsFormat(const std::string& extension) const {
  return supportedExtensions_.find(extension) != supportedExtensions_.end();
}

std::unique_ptr<Image> STBImageLoader::loadImage(const std::filesystem::path& filepath) {
  if (stbi_is_hdr(filepath.string().c_str())) {
    return loadImageData_(filepath, &loadHdr_, stbi_image_free, 32, true);
  } else if (stbi_is_16_bit(filepath.string().c_str())) {
    return loadImageData_(filepath, &load16Bit_, stbi_image_free, 16, false);
  } else {
    return loadImageData_(filepath, &load8Bit_, stbi_image_free, 8, false);
  }
}

void* STBImageLoader::loadHdr_(
    const std::filesystem::path& filepath, int32_t* x, int32_t* y, int32_t* channelsInFile, int32_t desiredChannels) {
  return static_cast<void*>(stbi_loadf(filepath.string().c_str(), x, y, channelsInFile, desiredChannels));
}

void* STBImageLoader::load16Bit_(
    const std::filesystem::path& filepath, int32_t* x, int32_t* y, int32_t* channelsInFile, int32_t desiredChannels) {
  return static_cast<void*>(stbi_load_16(filepath.string().c_str(), x, y, channelsInFile, desiredChannels));
}

void* STBImageLoader::load8Bit_(
    const std::filesystem::path& filepath, int32_t* x, int32_t* y, int32_t* channelsInFile, int32_t desiredChannels) {
  return static_cast<void*>(stbi_load(filepath.string().c_str(), x, y, channelsInFile, desiredChannels));
}

gfx::rhi::TextureFormat STBImageLoader::determineFormat_(int32_t channels, int32_t bitsPerChannel, bool isHdr) {
  if (channels == 1) {
    if (bitsPerChannel == 8 && !isHdr) {
      return gfx::rhi::TextureFormat::R8;
    } else if (bitsPerChannel == 16 && !isHdr) {
      return gfx::rhi::TextureFormat::R16f;
    } else if (bitsPerChannel == 32 || isHdr) {
      return gfx::rhi::TextureFormat::R32f;
    }
  } else if (channels == 2) {
    if (bitsPerChannel == 8 && !isHdr) {
      return gfx::rhi::TextureFormat::Rg8;
    } else if (bitsPerChannel == 16 && !isHdr) {
      return gfx::rhi::TextureFormat::Rg16f;
    } else if (bitsPerChannel == 32 || isHdr) {
      return gfx::rhi::TextureFormat::Rg32f;
    }
  } else if (channels == 3) {
    if (bitsPerChannel == 8 && !isHdr) {
      return gfx::rhi::TextureFormat::Rgb8;
    } else if (bitsPerChannel == 16 && !isHdr) {
      return gfx::rhi::TextureFormat::Rgb16f;
    } else if (bitsPerChannel == 32 || isHdr) {
      return gfx::rhi::TextureFormat::Rgb32f;
    }
  } else if (channels == 4) {
    if (bitsPerChannel == 8 && !isHdr) {
      return gfx::rhi::TextureFormat::Rgba8;
    } else if (bitsPerChannel == 16 && !isHdr) {
      return gfx::rhi::TextureFormat::Rgba16f;
    } else if (bitsPerChannel == 32 || isHdr) {
      return gfx::rhi::TextureFormat::Rgba32f;
    }
  }

  GlobalLogger::Log(LogLevel::Error,
                    "Unknown format. channels = " + std::to_string(channels) + ", bitsPerChannel = "
                        + std::to_string(bitsPerChannel) + ", isHdr = " + std::to_string(isHdr));
  return gfx::rhi::TextureFormat::Count;
}

std::unique_ptr<Image> STBImageLoader::loadImageData_(
    const std::filesystem::path& filepath, LoaderFunc loader, FreeFunc freeFunc, int32_t bitsPerChannel, bool isHdr) {
  int32_t width, height, channels;
  int32_t desiredChannels = 0;  // Load with the original number of channels

  auto* data = loader(filepath, &width, &height, &channels, desiredChannels);
  if (!data) {
    GlobalLogger::Log(LogLevel::Error, "Failed to load image: " + filepath.string());
    return nullptr;
  }

  size_t bytesPerChannel = bitsPerChannel / 8;
  size_t bytesPerPixel   = bytesPerChannel * channels;
  size_t imageSize       = static_cast<size_t>(width) * height * bytesPerPixel;

  std::vector<std::byte> pixels(reinterpret_cast<std::byte*>(data), reinterpret_cast<std::byte*>(data) + imageSize);

  freeFunc(data);

  auto image       = std::make_unique<Image>();
  image->width     = static_cast<size_t>(width);
  image->height    = static_cast<size_t>(height);
  image->depth     = 1;
  image->mipLevels = 1;
  image->arraySize = 1;
  image->dimension = gfx::rhi::TextureType::Texture2D;
  image->format    = determineFormat_(channels, bitsPerChannel, isHdr);
  image->pixels    = std::move(pixels);

  SubImage subImage;
  subImage.width      = image->width;
  subImage.height     = image->height;
  subImage.rowPitch   = image->width * bytesPerPixel;
  subImage.slicePitch = subImage.rowPitch * image->height;
  subImage.pixelBegin = image->pixels.begin();

  image->subImages.push_back(subImage);

  GlobalLogger::Log(LogLevel::Debug,
                    "Loaded image from " + filepath.string() + " (" + std::to_string(width) + "x"
                        + std::to_string(height) + ", channels = " + std::to_string(channels) + ", bitsPerChannel = "
                        + std::to_string(bitsPerChannel) + ", HDR = " + (isHdr ? "true" : "false") + ")");

  return image;
}

}  // namespace game_engine