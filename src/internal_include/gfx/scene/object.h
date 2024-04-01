// TODO:
// - move to more appropriate folder (e.g. scene)
// - consider moving scene folder to a more appropriate location (gfx is not the
// very best place for it)

#ifndef GAME_ENGINE_OBJECT_H
#define GAME_ENGINE_OBJECT_H

#include "gfx/scene/bound_primitive.h"
#include "gfx/scene/render_object.h"

#include <set>
#include <vector>

namespace game_engine {

class Object {
  public:
  Object() {}

  virtual ~Object() {
    Object::RemoveBoundBoxObject(BoundBoxObject);
    delete BoundBoxObject;

    Object::RemoveBoundSphereObject(BoundSphereObject);
    delete BoundSphereObject;

    for (auto& RenderObject : RenderObjects) {
      delete RenderObject;
    }
    RenderObjects.clear();
  }

  //////////////////////////////////////////////////////////////////////////
  static void AddObject(Object* object) {
    if (!object) {
      return;
    }

    // g_StaticObjectArray.push_back(object);

    if (!object->SkipShadowMapGen) {
      // if (object->RenderObject && object->RenderObject->VertexStream)
      //	JASSERT(object->RenderObject->VertexStream->PrimitiveType ==
      // EPrimitiveType::TRIANGLES);

      s_ShadowCasterObject.push_back(object);
    }
    s_StaticObjects.push_back(object);

    {
      for (auto& RenderObject : object->RenderObjects) {
        s_StaticRenderObjects.push_back(RenderObject);
        if (!object->SkipShadowMapGen) {
          s_ShadowCasterRenderObject.push_back(RenderObject);
        }
      }
    }
  }

  static void RemoveObject(Object* object) {
    if (!object) {
      return;
    }
    s_ShadowCasterObject.erase(
        std::remove_if(s_ShadowCasterObject.begin(),
                       s_ShadowCasterObject.end(),
                       [&object](Object* param) { return param == object; }));
    s_StaticObjects.erase(
        std::remove_if(s_StaticObjects.begin(),
                       s_StaticObjects.end(),
                       [&object](Object* param) { return param == object; }));

    {
      for (auto& renderObject : object->RenderObjects) {
        s_ShadowCasterRenderObject.erase(
            std::remove_if(s_ShadowCasterRenderObject.begin(),
                           s_ShadowCasterRenderObject.end(),
                           [&renderObject](RenderObject* param) {
                             return param == renderObject;
                           }));

        s_StaticRenderObjects.erase(
            std::remove_if(s_StaticRenderObjects.begin(),
                           s_StaticRenderObjects.end(),
                           [&renderObject](RenderObject* param) {
                             return param == renderObject;
                           }));
      }
    }
  }

  static void FlushDirtyState() {
    if (!s_DirtyStateObjects.empty()) {
      for (auto iter : s_DirtyStateObjects) {
        auto it_find = std::find(
            s_ShadowCasterObject.begin(), s_ShadowCasterObject.end(), iter);
        const bool existInShadowCasterObject
            = s_ShadowCasterObject.end() != it_find;
        // if (iter->SkipShadowMapGen)
        //{
        //	if (existInShadowCasterObject)
        //		s_ShadowCasterObject.erase(it_find);
        // }
        // else
        //{
        //	if (!existInShadowCasterObject)
        //		s_ShadowCasterObject.push_back(iter);
        // }
      }
    }
  }

  static const std::vector<Object*>& GetShadowCasterObject() {
    return s_ShadowCasterObject;
  }

  static const std::vector<Object*>& GetStaticObject() {
    return s_StaticObjects;
  }

  static const std::vector<Object*>& GetBoundBoxObject() {
    return s_BoundBoxObjects;
  }

  static const std::vector<Object*>& GetBoundSphereObject() {
    return s_BoundSphereObjects;
  }

  static const std::vector<Object*>& GetDebugObject() { return s_DebugObjects; }

  static const std::vector<Object*>& GetUIObject() { return s_UIObjects; }

  static const std::vector<Object*>& GetUIDebugObject() {
    return s_UIDebugObjects;
  }

  static const std::vector<RenderObject*>& GetShadowCasterRenderObject() {
    return s_ShadowCasterRenderObject;
  }

  static const std::vector<RenderObject*>& GetStaticRenderObject() {
    return s_StaticRenderObjects;
  }

  static void AddBoundBoxObject(Object* object) {
    if (!object) {
      return;
    }
    s_BoundBoxObjects.push_back(object);
  }

  static void RemoveBoundBoxObject(Object* object) {
    if (!object) {
      return;
    }
    s_BoundBoxObjects.erase(
        std::remove_if(s_BoundBoxObjects.begin(),
                       s_BoundBoxObjects.end(),
                       [&object](Object* param) { return param == object; }));
  }

  static void AddBoundSphereObject(Object* object) {
    if (!object) {
      return;
    }
    s_BoundSphereObjects.push_back(object);
  }

  static void RemoveBoundSphereObject(Object* object) {
    if (!object) {
      return;
    }
    s_BoundSphereObjects.erase(
        std::remove_if(s_BoundSphereObjects.begin(),
                       s_BoundSphereObjects.end(),
                       [&object](Object* param) { return param == object; }));
  }

  static void AddDebugObject(Object* object) {
    if (!object) {
      return;
    }
    s_DebugObjects.push_back(object);
  }

  static void RemoveDebugObject(Object* object) {
    if (!object) {
      return;
    }
    s_DebugObjects.erase(std::remove_if(
        s_DebugObjects.begin(), s_DebugObjects.end(), [&object](Object* param) {
          return param == object;
        }));
  }

  static void AddUIObject(Object* object) {
    if (!object) {
      return;
    }
    s_UIObjects.push_back(object);
  }

  static void RemoveUIObject(Object* object) {
    if (!object) {
      return;
    }
    s_UIObjects.erase(std::remove_if(
        s_UIObjects.begin(), s_UIObjects.end(), [&object](Object* param) {
          return param == object;
        }));
  }

  static void AddUIDebugObject(Object* object) {
    if (!object) {
      return;
    }
    s_UIDebugObjects.push_back(object);
  }

  static void RemoveUIDebugObject(Object* object) {
    if (!object) {
      return;
    }
    s_UIDebugObjects.erase(
        std::remove_if(s_UIDebugObjects.begin(),
                       s_UIDebugObjects.end(),
                       [&object](Object* param) { return param == object; }));
  }

  //////////////////////////////////////////////////////////////////////////

  virtual void Update(float deltaTime) {
    if (IsPostUpdate && PostUpdateFunc) {
      PostUpdateFunc(this, deltaTime);
    }
  }

  void SetDirtyState() {
    DirtyObjectState = true;
    s_DirtyStateObjects.insert(this);
  }

  void SetSkipShadowMapGen(bool skipShadowMapGen) {
    SkipShadowMapGen = skipShadowMapGen;
    SetDirtyState();
  }

  void SetSkipUpdateShadowVolume(bool skipUpdateShadowVolume) {
    SkipUpdateShadowVolume = skipUpdateShadowVolume;
    SetDirtyState();
  }

  void SetVisible(bool visible) {
    Visible = visible;
    SetDirtyState();
  }

  void ShowBoundBox(bool isShow) {
    if (isShow) {
      Object::AddBoundBoxObject(BoundBoxObject);
      Object::AddBoundSphereObject(BoundSphereObject);
    } else {
      Object::RemoveBoundBoxObject(BoundBoxObject);
      Object::RemoveBoundSphereObject(BoundSphereObject);
    }
  }

  bool HasInstancing() const {
    return RenderObjects[0] ? RenderObjects[0]->HasInstancing() : false;
  }

  std::shared_ptr<RenderObjectGeometryData> RenderObjectGeometryDataPtr;
  std::vector<RenderObject*>                RenderObjects;

  bool SkipShadowMapGen       = false;
  bool SkipUpdateShadowVolume = false;
  bool Visible                = true;
  bool DirtyObjectState       = false;

  bool                                IsPostUpdate = true;
  std::function<void(Object*, float)> PostUpdateFunc;

  Object* BoundBoxObject    = nullptr;
  Object* BoundSphereObject = nullptr;

  BoundBox    boundBox;
  BoundSphere boundSphere;

  // TODO: make private
  // private:
  static std::vector<Object*>       s_ShadowCasterObject;
  static std::vector<RenderObject*> s_ShadowCasterRenderObject;
  static std::vector<Object*>       s_StaticObjects;
  static std::vector<RenderObject*> s_StaticRenderObjects;
  static std::vector<Object*>       s_BoundBoxObjects;
  static std::vector<Object*>       s_BoundSphereObjects;
  static std::vector<Object*>       s_DebugObjects;
  static std::vector<Object*>       s_UIObjects;
  static std::vector<Object*>       s_UIDebugObjects;
  static std::set<Object*>          s_DirtyStateObjects;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_OBJECT_H