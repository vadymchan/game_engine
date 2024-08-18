// TODO:
// - move to more appropriate folder (e.g. scene)
// - consider moving scene folder to a more appropriate location (gfx is not the
// very best place for it)

#include "gfx/scene/object.h"

namespace game_engine {

   std::vector<Object*>       Object::s_shadowCasterObject;
   std::vector<RenderObject*> Object::s_shadowCasterRenderObject;
   std::vector<Object*>       Object::s_staticObjects;
   std::vector<RenderObject*> Object::s_staticRenderObjects;
   std::vector<Object*>       Object::s_boundBoxObjects;
   std::vector<Object*>       Object::s_boundSphereObjects;
   std::vector<Object*>       Object::s_debugObjects;
   std::vector<Object*>       Object::s_UIObjects;
   std::vector<Object*>       Object::s_UIDebugObjects;
   std::set<Object*>          Object::s_dirtyStateObjects;


}  // namespace game_engine
