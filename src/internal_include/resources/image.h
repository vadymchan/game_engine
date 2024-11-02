#ifndef GAME_ENGINE_IMAGE_H
#define GAME_ENGINE_IMAGE_H

#include <cstdint>
#include <vector>

namespace game_engine {

struct Image {
  int32_t                width;
  int32_t                height;
  // TODO: use format  instead of channels, bitsPerChannel and isHdr
  int32_t                channels;
  int32_t                bitsPerChannel;
  bool                   isHdr;
  std::vector<std::byte> data;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_IMAGE_H
