#ifndef GAME_ENGINE_IMAGE_H
#define GAME_ENGINE_IMAGE_H

#include <cstdint>
#include <vector>

namespace game_engine {

struct SubImage {
  size_t width;
  size_t height;
  size_t rowPitch;    // width * height * bytes per pixel (also known as stride)
  size_t slicePitch;  // width * height * bytes per pixel
  // TODO: consider renaming
  std::vector<std::byte>::const_iterator pixelBegin;  // Start of pixel data
};

struct Image {
  size_t width;
  size_t height;
  size_t depth;      // For 3D textures; 1 for 2D textures
  size_t mipLevels;  // Number of mipmap levels; 1 if no mipmaps
  size_t arraySize;  // Number of array slices; 1 if not an array texture
  // TODO: consider renaming Texture... to something else (maybe PixelFormat and
  // ImageDimension - alias to current enums)
  ETextureFormat format;     //
  ETextureType   dimension;  // Texture dimension - 1D, 2D, 3D

  // Raw pixel data for all mip levels and array slices
  std::vector<std::byte> pixels;

  // Holds sub-images for each mip level / array slice
  std::vector<SubImage> subImages;
};

// TODO: consider moving to separate file (like image_type.h)
#include <algorithm>
#include <string>
#include <unordered_map>

enum class EImageType {
  JPEG,
  JPG,
  PNG,
  BMP,
  TGA,
  GIF,
  HDR,
  PIC,
  PPM,
  PGM,
  DDS,
  UNKNOWN,
};

inline EImageType getImageTypeFromExtension(const std::string& extension) {
  static const std::unordered_map<std::string, EImageType> extensionToType = {
    {".jpeg", EImageType::JPEG},
    { ".jpg",  EImageType::JPG},
    { ".png",  EImageType::PNG},
    { ".bmp",  EImageType::BMP},
    { ".tga",  EImageType::TGA},
    { ".gif",  EImageType::GIF},
    { ".hdr",  EImageType::HDR},
    { ".pic",  EImageType::PIC},
    { ".ppm",  EImageType::PPM},
    { ".pgm",  EImageType::PGM},
    { ".dds",  EImageType::DDS},
  };

  std::string ext = extension;
  std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

  auto it = extensionToType.find(ext);
  if (it != extensionToType.end()) {
    return it->second;
  }

  return EImageType::UNKNOWN;
}

}  // namespace game_engine

#endif  // GAME_ENGINE_IMAGE_H
