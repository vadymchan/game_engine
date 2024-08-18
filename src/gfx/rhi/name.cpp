#include "gfx/rhi/name.h"

namespace game_engine {

std::unordered_map<uint32_t, std::shared_ptr<std::string>> Name::s_nameTable;
MutexRWLock                                                Name::s_lock;
const Name                                                 Name::s_kInvalid;

}  // namespace game_engine