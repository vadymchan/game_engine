// TODO:
// - move to more appropriate folder (e.g. scene)
// - consider moving scene folder to a more appropriate location (gfx is not the
// very best place for it)

#include "gfx/scene/object.h"

namespace game_engine {

   std::vector<Object*>       Object::s_ShadowCasterObject;
   std::vector<RenderObject*> Object::s_ShadowCasterRenderObject;
   std::vector<Object*>       Object::s_StaticObjects;
   std::vector<RenderObject*> Object::s_StaticRenderObjects;
   std::vector<Object*>       Object::s_BoundBoxObjects;
   std::vector<Object*>       Object::s_BoundSphereObjects;
   std::vector<Object*>       Object::s_DebugObjects;
   std::vector<Object*>       Object::s_UIObjects;
   std::vector<Object*>       Object::s_UIDebugObjects;
   std::set<Object*>          Object::s_DirtyStateObjects;


}  // namespace game_engine
