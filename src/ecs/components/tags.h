#ifndef GAME_ENGINE_TAGS_H
#define GAME_ENGINE_TAGS_H

#include <filesystem>

namespace game_engine {

struct ModelLoadingTag {
  std::filesystem::path modelPath;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_TAGS_H