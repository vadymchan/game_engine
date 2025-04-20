#ifndef GAME_ENGINE_ALIGN_H
#define GAME_ENGINE_ALIGN_H

#include <cassert>
#include <cstddef>
#include <type_traits>

namespace game_engine {

/**
 * Graphics APIs like DirectX 12 and Vulkan require constant buffers to be aligned
 * to 256-byte boundaries. This function rounds up any size to the next multiple of 256.
 *
 * @param size Original buffer size in bytes
 * @return Size aligned to 256-byte boundary
 */
inline uint64_t alignConstantBufferSize(uint64_t size) {
  return (size + 255) & ~255;
}

}  // namespace game_engine

#endif  // GAME_ENGINE_ALIGN_H