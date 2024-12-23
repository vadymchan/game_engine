#include "utils/third_party/directx_tex_util.h"

#include "gfx/rhi/dx12/rhi_type_dx12.h"

#include <algorithm>
#include <iostream>
#include <unordered_set>

namespace game_engine {

const std::unordered_set<std::string>
    DirectXTexImageLoader::supportedExtensions_ = {".dds"};

bool DirectXTexImageLoader::supportsFormat(const std::string& extension) const {
  return supportedExtensions_.find(extension) != supportedExtensions_.end();
}

ETextureType DirectXTexImageLoader::determineDimension_(
    const DirectX::TexMetadata& metadata) const {
  ETextureType baseDimension;
  switch (metadata.dimension) {
    case DirectX::TEX_DIMENSION_TEXTURE1D:
      baseDimension = ETextureType::TEXTURE_1D;
      break;
    case DirectX::TEX_DIMENSION_TEXTURE2D:
      baseDimension = (metadata.IsCubemap())   ? ETextureType::TEXTURE_CUBE
                    : (metadata.arraySize > 1) ? ETextureType::TEXTURE_2D_ARRAY
                                               : ETextureType::TEXTURE_2D;
      break;
    case DirectX::TEX_DIMENSION_TEXTURE3D:
      baseDimension = (metadata.arraySize > 1) ? ETextureType::TEXTURE_3D_ARRAY
                                               : ETextureType::TEXTURE_3D;
      break;
    default:
      // TODO: add logger
      std::cerr << "Unsupported texture dimension. Defaulting to TEXTURE_2D.\n";
      baseDimension = ETextureType::MAX;
      break;
  }
  return baseDimension;
}

std::shared_ptr<Image> DirectXTexImageLoader::loadImage(
    const std::filesystem::path& filepath) {
  DirectX::TexMetadata  metadata;
  DirectX::ScratchImage scratchImage;

  HRESULT hr = DirectX::LoadFromDDSFile(filepath.wstring().c_str(),
                                        DirectX::DDS_FLAGS_NONE,
                                        &metadata,
                                        scratchImage);

  if (FAILED(hr)) {
    // TODO: add logger
    std::cerr << "Failed to load DDS image: " << filepath << ". HRESULT: " << hr
              << std::endl;
    return nullptr;
  }

  // Calculate total pixel data size
  size_t totalBytes = scratchImage.GetPixelsSize();

  // Copy pixel data into Image::pixels
  std::vector<std::byte> pixels(
      reinterpret_cast<std::byte*>(scratchImage.GetPixels()),
      reinterpret_cast<std::byte*>(scratchImage.GetPixels()) + totalBytes);

  // Populate Image struct
  std::shared_ptr<Image> image = std::make_shared<Image>();
  image->width                 = metadata.width;
  image->height                = metadata.height;
  image->depth                 = metadata.depth;
  image->mipLevels             = metadata.mipLevels;
  image->arraySize             = metadata.arraySize;
  image->format                = g_getDX12TextureFormat(metadata.format);
  image->dimension             = determineDimension_(metadata);
  image->pixels                = std::move(pixels);

  // Populate SubImages
  image->subImages.reserve(image->mipLevels * image->arraySize * image->depth);

  for (auto mip = 0; mip < image->mipLevels; ++mip) {
    for (auto arraySlice = 0; arraySlice < image->arraySize; ++arraySlice) {
      for (auto slice = 0; slice < image->depth; ++slice) {
        const auto* img = scratchImage.GetImage(mip, arraySlice, slice);
        if (!img) {
          // TODO: add logger
          std::cerr << "Failed to retrieve image data for mip level " << mip
                    << ", array slice " << arraySlice << ", depth slice "
                    << slice << std::endl;
          continue;
        }

        SubImage subImage;
        subImage.width      = img->width;
        subImage.height     = img->height;
        subImage.rowPitch   = img->rowPitch;
        subImage.slicePitch = img->slicePitch;
        subImage.pixelBegin
            = image->pixels.begin() + (img->pixels - scratchImage.GetPixels());

        image->subImages.emplace_back(subImage);
      }
    }
  }

  return image;
}

}  // namespace game_engine
