#ifndef ARISE_ALIGN_H
#define ARISE_ALIGN_H

#include <cassert>
#include <cstddef>
#include <type_traits>

namespace arise {

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

}  // namespace arise

#endif  // ARISE_ALIGN_H