#include "utils/string/string_conversion.h"

// TODO: if possible, replace win api with more portable solution
#include <windows.h>

namespace game_engine {

std::wstring ConvertToWchar(const char* InPath, int32_t InLength) {
  assert(InPath);

  std::wstring result;
  if (InLength > 0) {
    result.resize(InLength + 1);
    {
      const int32_t ResultFilePathLength
          = MultiByteToWideChar(CP_ACP, 0, InPath, -1, NULL, NULL);
      assert(ResultFilePathLength < 256);

      MultiByteToWideChar(CP_ACP,
                          0,
                          InPath,
                          InLength,
                          &result[0],
                          (int32_t)(result.size() - 1));
      result[InLength] = 0;
    }
  }
  return result;
}

}  // namespace game_engine
