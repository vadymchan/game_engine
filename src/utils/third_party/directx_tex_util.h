#ifndef GAME_ENGINE_DIRECTX_TEX_UTILS_H
#define GAME_ENGINE_DIRECTX_TEX_UTILS_H

#include "file_loader/image_file_loader.h"
#include "gfx/rhi/common/rhi_enums.h"

#include <DirectXTex.h>

#include <filesystem>
#include <unordered_set>
#include <vector>

namespace game_engine {

class DirectXTexImageLoader : public IImageLoader {
  public:
  std::unique_ptr<Image> loadImage(const std::filesystem::path& filepath) override;

  bool supportsFormat(const std::string& extension) const override;

  private:
  gfx::rhi::TextureType determineDimension_(const DirectX::TexMetadata& metadata) const;

  static const std::unordered_set<std::string> supportedExtensions_;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_DIRECTX_TEX_UTILS_H