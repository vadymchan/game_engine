#ifndef ARISE_TAGS_H
#define ARISE_TAGS_H

#include <filesystem>

namespace arise {

struct ModelLoadingTag {
  std::filesystem::path modelPath;
};

}  // namespace arise

#endif  // ARISE_TAGS_H