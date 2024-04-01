#include "gfx/rhi/name.h"

namespace game_engine {

std::unordered_map<uint32_t, std::shared_ptr<std::string>> Name::s_NameTable;
MutexRWLock                                                Name::Lock;
const Name                                                 Name::Invalid;

}  // namespace game_engine