#define SDL_MAIN_HANDLED

#include "engine.h"

// ----------------------------------------------
// Event system
#include "event/event.h"
#include "event/event_handler.h"
#include "event/keyboard_event_handler.h"
#include "event/mouse_event_handler.h"
#include "event/window_event_handler.h"
#include "event/window_event_manager.h"

// Input system
#include "input/input_manager.h"
#include "input/key.h"
#include "input/mouse.h"

// Platform specific
#include "platform/common/window.h"

// Utilities
#include "gfx/rhi/vulkan/spirv_util.h"
#include "utils/logger/console_logger.h"
#include "utils/logger/global_logger.h"
#include "utils/logger/i_logger.h"
#include "utils/time/stopwatch.h"

// ----------------------------------------------

#include "utils/image/image_loader_manager.h"
#include "utils/image/image_manager.h"
#include "utils/third_party/directx_tex_util.h"
#include "utils/third_party/stb_util.h"
// #include "resources/image.h"
// #include "file_loader/image_file_loader.h"

// #include <climits>
// #include <cstddef>
// #include <cstdint>
// #include <filesystem>
// #include <ecs/systems/render_system.h>
// #include <ecs/systems/system_manager.h>
#include <resources/assimp_material_loader.h>
// #include <resources/assimp_model_loader.h>
// #include <resources/assimp_render_model_loader.h>

#include <fstream>

using namespace game_engine;

// #include <functional>
// #include <iostream>
// #include <memory>
// #include <unordered_map>
// #include <vector>
//
// #define STB_IMAGE_IMPLEMENTATION
// #include <stb_image.h>

// Image structure to hold image data
// struct Image {
//  int32_t                width;
//  int32_t                height;
//  int32_t                channels;
//  int32_t                bitsPerChannel;
//  bool                   isHdr;
//  std::vector<std::byte> data;
//};
//
//// Image Loader Interface
// class IImageLoader {
//   public:
//   virtual ~IImageLoader() = default;
//   virtual std::shared_ptr<Image> loadImage(
//       const std::filesystem::path& filepath)
//       = 0;
// };
//
//// Implementation using STB Image
// class STBImageLoader : public IImageLoader {
//   public:
//   std::shared_ptr<Image> loadImage(
//       const std::filesystem::path& filepath) override {
//     if (stbi_is_hdr(filepath.string().c_str())) {
//       return loadImageData_(filepath, &loadHdr_, stbi_image_free, 32, true);
//     } else if (stbi_is_16_bit(filepath.string().c_str())) {
//       return loadImageData_(filepath, &load16Bit_, stbi_image_free, 16,
//       false);
//     } else {
//       return loadImageData_(filepath, &load8Bit_, stbi_image_free, 8, false);
//     }
//   }
//
//   private:
//   using LoaderFunc = std::function<void*(
//       const std::filesystem::path&, int32_t*, int32_t*, int32_t*, int32_t)>;
//   using FreeFunc   = std::function<void(void*)>;
//
//   // TODO: consider adding desired parameter for choosing channels for the
//   // functions below
//   static void* loadHdr_(const std::filesystem::path& filepath,
//                         int32_t*                     x,
//                         int32_t*                     y,
//                         int32_t*                     channelsInFile,
//                         int32_t                      desiredChannels = 0) {
//     return static_cast<void*>(stbi_loadf(
//         filepath.string().c_str(), x, y, channelsInFile, desiredChannels));
//   }
//
//   static void* load16Bit_(const std::filesystem::path& filepath,
//                           int32_t*                     x,
//                           int32_t*                     y,
//                           int32_t*                     channelsInFile,
//                           int32_t                      desiredChannels = 0) {
//     return static_cast<void*>(stbi_load_16(
//         filepath.string().c_str(), x, y, channelsInFile, desiredChannels));
//   }
//
//   static void* load8Bit_(const std::filesystem::path& filepath,
//                          int32_t*                     x,
//                          int32_t*                     y,
//                          int32_t*                     channelsInFile,
//                          int32_t                      desiredChannels = 0) {
//     return static_cast<void*>(stbi_load(
//         filepath.string().c_str(), x, y, channelsInFile, desiredChannels));
//   }
//
//   std::shared_ptr<Image> loadImageData_(const std::filesystem::path&
//   filepath,
//                                         LoaderFunc                   loader,
//                                         FreeFunc freeFunc, int32_t
//                                         bitsPerChannel, bool    isHdr) {
//     int32_t width, height, channels;
//     // TODO: make it parameterized
//     int32_t desiredChannels = 0;
//
//     auto* data = loader(filepath, &width, &height, &channels,
//     desiredChannels); if (!data) {
//       std::cerr << "Failed to load image: " << filepath << std::endl;
//       return nullptr;
//     }
//
//     const size_t imageSize = static_cast<size_t>(width) * height * channels
//                            * (bitsPerChannel / CHAR_BIT);
//
//     std::vector<std::byte> imageData(
//         reinterpret_cast<std::byte*>(data),
//         reinterpret_cast<std::byte*>(data) + imageSize);
//
//     freeFunc(data);
//     return std::make_shared<Image>(Image{
//       width, height, channels, bitsPerChannel, isHdr, std::move(imageData)});
//   }
// };
//
//// Resource Manager for handling image loading and caching
// class ImageManager {
//   public:
//   ImageManager(std::shared_ptr<IImageLoader> loader)
//       : m_loader_(std::move(loader)) {}
//
//   std::shared_ptr<Image> getImage(const std::filesystem::path& filepath) {
//     auto it = m_image_cache_.find(filepath);
//     if (it != m_image_cache_.end()) {
//       return it->second;
//     }
//
//     auto image = m_loader_->loadImage(filepath);
//     if (image) {
//       m_image_cache_[filepath] = image;
//       return image;
//     }
//     return nullptr;
//   }
//
//   private:
//   std::shared_ptr<IImageLoader> m_loader_;
//   std::unordered_map<std::filesystem::path, std::shared_ptr<Image>>
//       m_image_cache_;
// };

#if (defined(_WIN32) || defined(_WIN64)) \
    && defined(GAME_ENGINE_WINDOWS_SUBSYSTEM)
#include <windows.h>
int WINAPI wWinMain(_In_ HINSTANCE     hInstance,
                    _In_opt_ HINSTANCE hPrevInstance,
                    _In_ PWSTR         pCmdLine,
                    _In_ int           nCmdShow) {
#else
auto main(int argc, char* argv[]) -> int {

#endif

  // Inform SDL that the program will handle its own initialization
  SDL_SetMainReady();

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
    SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
    return EXIT_FAILURE;
  }

  game_engine::Engine engine;

  engine.init();

  auto game = std::make_shared<game_engine::Game>();

  engine.setGame(game);

  game->setup();

  engine.run();

  engine.release();

  // TODO: move to other place
  game_engine::g_rhi->release();

  SDL_Quit();

  // std::shared_ptr<IImageLoader> loader =
  // std::make_shared<STBImageLoader>();
  // ImageManager                  imageManager(loader);

  //// auto image = imageManager.getImage("assets/images/beauty.png");
  //// if (image) {
  ////   /*std::cout << "Image loaded with dimensions: " << image->width <<
  // "x"
  ////             << image->height << ", channels: " << image->channels
  ////             << ", bits per channel: " << image->bitsPerChannel
  ////             << (image->isHdr ? " (HDR)" : "") << std::endl;*/

  ////  for (auto element : image->data) {
  ////    std::cout << static_cast<int>(element) << " ";
  ////  }
  ////}

  // auto image = imageManager.getImage("assets/images/Head_albedo.dds");
  // if (image) {
  //   std::ofstream outFile("image_data.txt");  // Открываем файл для
  //   записи

  //  if (outFile.is_open()) {
  //    outFile << "Image loaded with dimensions: " << image->width << "x"
  //            << image->height << ", channels: " << image->channels
  //            << ", bits per channel: " << image->bitsPerChannel
  //            << (image->isHdr ? " (HDR)" : "") << std::endl;

  //    for (auto element : image->data) {
  //      outFile << static_cast<int>(element) << " ";
  //    }
  //    outFile.close();  // Закрываем файл после записи
  //  } else {
  //    std::cerr << "Failed to open file for writing." << std::endl;
  //  }
  //}

  return EXIT_SUCCESS;
}
//
// #include <DirectXTex.h>
//
// #include <iostream>
//
// using namespace DirectX;
//
// namespace MyNamespace {
//
// struct TexMetadata {
//   size_t        width;
//   size_t        height;     // Should be 1 for 1D textures
//   size_t        depth;      // Should be 1 for 1D or 2D textures
//   size_t        arraySize;  // For cubemap, this is a multiple of 6
//   size_t        mipLevels;
//   uint32_t      miscFlags;
//   uint32_t      miscFlags2;
//   DXGI_FORMAT   format;
//   TEX_DIMENSION dimension;
//
//   size_t __cdecl ComputeIndex(size_t mip,
//                               size_t item,
//                               size_t slice) const noexcept;
//
//   // Returns size_t(-1) to indicate an out-of-range error
//
//   bool __cdecl IsCubemap() const noexcept {
//     return (miscFlags & TEX_MISC_TEXTURECUBE) != 0;
//   }
//
//   // Helper for miscFlags
//
//   bool __cdecl IsPMAlpha() const noexcept {
//     return ((miscFlags2 & TEX_MISC2_ALPHA_MODE_MASK)
//             == TEX_ALPHA_MODE_PREMULTIPLIED)
//         != 0;
//   }
//
//   void __cdecl SetAlphaMode(TEX_ALPHA_MODE mode) noexcept {
//     miscFlags2
//         = (miscFlags2 & ~static_cast<uint32_t>(TEX_MISC2_ALPHA_MODE_MASK))
//         | static_cast<uint32_t>(mode);
//   }
//
//   TEX_ALPHA_MODE __cdecl GetAlphaMode() const noexcept {
//     return static_cast<TEX_ALPHA_MODE>(miscFlags2 &
//     TEX_MISC2_ALPHA_MODE_MASK);
//   }
//
//   // Helpers for miscFlags2
//
//   bool __cdecl IsVolumemap() const noexcept {
//     return dimension == TEX_DIMENSION_TEXTURE3D;
//   }
//
//   // Helper for dimension
//
//   uint32_t __cdecl CalculateSubresource(size_t mip, size_t item) const
//   noexcept; uint32_t __cdecl CalculateSubresource(size_t mip,
//                                         size_t item,
//                                         size_t plane) const noexcept;
//   // Returns size_t(-1) to indicate an out-of-range error
// };
//
// struct Image {
//   size_t      width;
//   size_t      height;
//   DXGI_FORMAT format;
//   size_t      rowPitch;
//   size_t      slicePitch;
//   uint8_t*    pixels;
// };
//
// class ScratchImage {
//   private:
//   size_t      m_nimages;
//   size_t      m_size;
//   TexMetadata m_metadata;
//   Image*      m_image;
//   uint8_t*    m_memory;
// };
//
// }  // namespace MyNamespace
//
// namespace Solution {
//
// struct SubImage {
//   size_t width;
//   size_t height;
//   size_t rowPitch;    // width * height * bytes per pixel (also known as
//   stride) size_t slicePitch;  // width * height * bytes per pixel
//   std::vector<std::byte>::const_iterator pixelBegin;  // Start of pixel data
// };
//
// struct Image {
//   size_t width;
//   size_t height;
//   size_t depth;      // For 3D textures; 1 for 2D textures
//   size_t mipLevels;  // Number of mipmap levels; 1 if no mipmaps
//   size_t arraySize;  // Number of array slices; 1 if not an array texture
//   // TODO: consider renaming Texture... to something else (maybe PixelFormat
//   and
//   // ImageDimension - alias to current enums)
//   ETextureFormat format;     //
//   ETextureType   dimension;  // Texture dimension - 1D, 2D, 3D
//
//   // Raw pixel data for all mip levels and array slices
//   std::vector<std::byte> pixels;
//
//   // Holds sub-images for each mip level / array slice
//   std::vector<SubImage> subImages;
// };
//
// }  // namespace Solution

// int main() {
//   using namespace game_engine;
//   // Инициализация COM
//   // HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
//   // if (FAILED(hr)) {
//   //  std::wcerr << L"Failed to initialize COM." << std::endl;
//   //  return -1;
//   //}
//
//   auto imageLoaderManager = std::make_shared<ImageLoaderManager>();
//
//   // Create loader instances
//   auto stbLoader        = std::make_shared<STBImageLoader>();
//   auto directxTexLoader = std::make_shared<DirectXTexImageLoader>();
//
//   // Register image types for stbLoader
//   imageLoaderManager->registerLoader(EImageType::JPEG, stbLoader);
//   imageLoaderManager->registerLoader(EImageType::JPG, stbLoader);
//   imageLoaderManager->registerLoader(EImageType::PNG, stbLoader);
//   imageLoaderManager->registerLoader(EImageType::BMP, stbLoader);
//   imageLoaderManager->registerLoader(EImageType::TGA, stbLoader);
//   imageLoaderManager->registerLoader(EImageType::GIF, stbLoader);
//   imageLoaderManager->registerLoader(EImageType::HDR, stbLoader);
//   imageLoaderManager->registerLoader(EImageType::PIC, stbLoader);
//   imageLoaderManager->registerLoader(EImageType::PPM, stbLoader);
//   imageLoaderManager->registerLoader(EImageType::PGM, stbLoader);
//
//   // Register image types for directxTexLoader
//   imageLoaderManager->registerLoader(EImageType::DDS, directxTexLoader);
//
//   // Provide ImageLoaderManager via ServiceLocator
//   ServiceLocator::s_provide<ImageLoaderManager>(imageLoaderManager);
//
//   // Provide ImageManager via ServiceLocator
//   auto imageManager = std::make_shared<ImageManager>();
//   ServiceLocator::s_provide<ImageManager>(imageManager);
//
//   auto image1 = imageManager->getImage("assets/images/Head_albedo.dds");
//
//   //"assets/models/cube/cube.fbx"
//
//   // auto systemManager = std::make_shared<SystemManager>();
//
//   // systemManager->addSystem(std::make_shared<RenderSystem>());
//
//   // auto scene = std::make_shared<Scene>();
//
//   // auto& registry = scene->getEntityRegistry();
//
//   // std::shared_ptr<IModelLoader> modelLoader
//   //     = std::make_shared<AssimpModelLoader>();
//
//   std::shared_ptr<AssimpMaterialLoader> materialLoader
//       = std::make_shared<AssimpMaterialLoader>();
//
//   // std::shared_ptr<IRenderModelLoader> renderModelLoader
//   //     = std::make_shared<AssimpRenderModelLoader>();
//
//   // auto model = modelLoader->loadModel("assets/models/cube/cube.fbx");
//   auto materials =
//   materialLoader->loadMaterials("assets/models/cube/cube.fbx");
//
//   // systemManager->updateSystems(scene, 0.5);
//
//   return 0;
// }
