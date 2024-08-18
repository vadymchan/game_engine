// TODO:
// - move to more appropriate folder (e.g. scene)
// - consider moving scene folder to a more appropriate location (gfx is not the
// very best place for it)



#include "gfx/scene/camera.h"

namespace game_engine {

  std::map<int, Camera*> Camera::s_cameraMap;

}  // namespace game_engine
