#include "utils/string/string_conversion.h"

// TODO: if possible, replace win api with more portable solution
#include <windows.h>

namespace game_engine {

std::wstring g_convertToWchar(const char* path, int32_t length) {
  assert(path);

  std::wstring result;
  if (length > 0) {
    result.resize(length + 1);
    {
      const int32_t ResultFilePathLength
          = MultiByteToWideChar(CP_ACP, 0, path, -1, NULL, NULL);
      assert(ResultFilePathLength < 256);

      MultiByteToWideChar(CP_ACP,
                          0,
                          path,
                          length,
                          &result[0],
                          (int32_t)(result.size() - 1));
      result[length] = 0;
    }
  }
  return result;
}

}  // namespace game_engine
