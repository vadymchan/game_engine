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
  // ======= BEGIN: public static methods =====================================

  static const std::vector<Object*>& s_getShadowCasterObject() {
    return s_shadowCasterObject;
  }

  static const std::vector<Object*>& s_getStaticObject() {
    return s_staticObjects;
  }

  static const std::vector<Object*>& s_getBoundBoxObject() {
    return s_boundBoxObjects;
  }

  static const std::vector<Object*>& s_getBoundSphereObject() {
    return s_boundSphereObjects;
  }

  static const std::vector<Object*>& s_getDebugObject() {
    return s_debugObjects;
  }

  static const std::vector<Object*>& s_getUIObject() { return s_UIObjects; }

  static const std::vector<Object*>& s_getUIDebugObject() {
    return s_UIDebugObjects;
  }

  static const std::vector<RenderObject*>& s_getShadowCasterRenderObject() {
    return s_shadowCasterRenderObject;
  }

  static const std::vector<RenderObject*>& s_getStaticRenderObject() {
    return s_staticRenderObjects;
  }

  static void s_addObject(Object* object) {
    if (!object) {
      return;
    }

    // g_StaticObjectArray.push_back(object);

    if (!object->m_skipShadowMapGen_) {
      // if (object->renderObject && object->renderObject->VertexStream)
      //	JASSERT(object->renderObject->VertexStream->PrimitiveType ==
      // EPrimitiveType::TRIANGLES);

      s_shadowCasterObject.push_back(object);
    }
    s_staticObjects.push_back(object);

    {
      for (auto& renderObject : object->m_renderObjects_) {
        s_staticRenderObjects.push_back(renderObject);
        if (!object->m_skipShadowMapGen_) {
          s_shadowCasterRenderObject.push_back(renderObject);
        }
      }
    }
  }

  static void s_addBoundBoxObject(Object* object) {
    if (!object) {
      return;
    }
    s_boundBoxObjects.push_back(object);
  }

  static void s_addBoundSphereObject(Object* object) {
    if (!object) {
      return;
    }
    s_boundSphereObjects.push_back(object);
  }

  static void s_addDebugObject(Object* object) {
    if (!object) {
      return;
    }
    s_debugObjects.push_back(object);
  }

  static void s_addUIObject(Object* object) {
    if (!object) {
      return;
    }
    s_UIObjects.push_back(object);
  }

  static void s_addUIDebugObject(Object* object) {
    if (!object) {
      return;
    }
    s_UIDebugObjects.push_back(object);
  }

  static void s_removeObject(Object* object) {
    if (!object) {
      return;
    }
    s_shadowCasterObject.erase(
        std::remove_if(s_shadowCasterObject.begin(),
                       s_shadowCasterObject.end(),
                       [&object](Object* param) { return param == object; }));
    s_staticObjects.erase(
        std::remove_if(s_staticObjects.begin(),
                       s_staticObjects.end(),
                       [&object](Object* param) { return param == object; }));

    {
      for (auto& renderObject : object->m_renderObjects_) {
        s_shadowCasterRenderObject.erase(
            std::remove_if(s_shadowCasterRenderObject.begin(),
                           s_shadowCasterRenderObject.end(),
                           [&renderObject](RenderObject* param) {
                             return param == renderObject;
                           }));

        s_staticRenderObjects.erase(
            std::remove_if(s_staticRenderObjects.begin(),
                           s_staticRenderObjects.end(),
                           [&renderObject](RenderObject* param) {
                             return param == renderObject;
                           }));
      }
    }
  }

  static void s_removeBoundBoxObject(Object* object) {
    if (!object) {
      return;
    }
    s_boundBoxObjects.erase(
        std::remove_if(s_boundBoxObjects.begin(),
                       s_boundBoxObjects.end(),
                       [&object](Object* param) { return param == object; }));
  }

  static void s_removeBoundSphereObject(Object* object) {
    if (!object) {
      return;
    }
    s_boundSphereObjects.erase(
        std::remove_if(s_boundSphereObjects.begin(),
                       s_boundSphereObjects.end(),
                       [&object](Object* param) { return param == object; }));
  }

  static void s_removeDebugObject(Object* object) {
    if (!object) {
      return;
    }
    s_debugObjects.erase(std::remove_if(
        s_debugObjects.begin(), s_debugObjects.end(), [&object](Object* param) {
          return param == object;
        }));
  }

  static void s_removeUIObject(Object* object) {
    if (!object) {
      return;
    }
    s_UIObjects.erase(std::remove_if(
        s_UIObjects.begin(), s_UIObjects.end(), [&object](Object* param) {
          return param == object;
        }));
  }

  static void s_removeUIDebugObject(Object* object) {
    if (!object) {
      return;
    }
    s_UIDebugObjects.erase(
        std::remove_if(s_UIDebugObjects.begin(),
                       s_UIDebugObjects.end(),
                       [&object](Object* param) { return param == object; }));
  }

  static void s_flushDirtyState() {
    if (!s_dirtyStateObjects.empty()) {
      for (auto iter : s_dirtyStateObjects) {
        auto it_find = std::find(
            s_shadowCasterObject.begin(), s_shadowCasterObject.end(), iter);
        const bool existInShadowCasterObject
            = s_shadowCasterObject.end() != it_find;
        // if (iter->m_skipShadowMapGen_)
        //{
        //	if (existInShadowCasterObject)
        //		s_shadowCasterObject.erase(it_find);
        // }
        // else
        //{
        //	if (!existInShadowCasterObject)
        //		s_shadowCasterObject.push_back(iter);
        // }
      }
    }
  }

  // ======= END: public static methods   =====================================

  // ======= BEGIN: public static fields ======================================

  static std::vector<Object*>       s_shadowCasterObject;
  static std::vector<RenderObject*> s_shadowCasterRenderObject;
  static std::vector<Object*>       s_staticObjects;
  static std::vector<RenderObject*> s_staticRenderObjects;
  static std::vector<Object*>       s_boundBoxObjects;
  static std::vector<Object*>       s_boundSphereObjects;
  static std::vector<Object*>       s_debugObjects;
  // TODO: consider rename
  static std::vector<Object*>       s_UIObjects;
  static std::vector<Object*>       s_UIDebugObjects;
  static std::set<Object*>          s_dirtyStateObjects;

  // ======= END: public static fields   ======================================

  // ======= BEGIN: public constructors =======================================

  Object() {}

  // ======= END: public constructors   =======================================

  // ======= BEGIN: public destructor =========================================

  virtual ~Object() {
    Object::s_removeBoundBoxObject(m_boundBoxObject_);
    delete m_boundBoxObject_;

    Object::s_removeBoundSphereObject(m_boundSphereObject_);
    delete m_boundSphereObject_;

    for (auto& RenderObject : m_renderObjects_) {
      delete RenderObject;
    }
    m_renderObjects_.clear();
  }

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public setters ============================================

  void setDirtyState() {
    m_dirtyObjectState_ = true;
    s_dirtyStateObjects.insert(this);
  }

  void setSkipShadowMapGen(bool skipShadowMapGen) {
    m_skipShadowMapGen_ = skipShadowMapGen;
    setDirtyState();
  }

  void setSkipUpdateShadowVolume(bool skipUpdateShadowVolume) {
    m_skipUpdateShadowVolume_ = skipUpdateShadowVolume;
    setDirtyState();
  }

  void setVisible(bool visible) {
    m_isVisible_ = visible;
    setDirtyState();
  }

  // ======= END: public setters   ============================================

  // ======= BEGIN: public misc methods =======================================

  bool hasInstancing() const {
    return m_renderObjects_[0] ? m_renderObjects_[0]->hasInstancing() : false;
  }

  virtual void update(float deltaTime) {
    if (m_isPostUpdate_ && m_postUpdateFunc_) {
      m_postUpdateFunc_(this, deltaTime);
    }
  }

  void showBoundBox(bool isShow) {
    if (isShow) {
      Object::s_addBoundBoxObject(m_boundBoxObject_);
      Object::s_addBoundSphereObject(m_boundSphereObject_);
    } else {
      Object::s_removeBoundBoxObject(m_boundBoxObject_);
      Object::s_removeBoundSphereObject(m_boundSphereObject_);
    }
  }

  // ======= END: public misc methods   =======================================


  // TODO: make private
  // private:



  // ======= BEGIN: public misc fields ========================================

  std::shared_ptr<RenderObjectGeometryData> m_renderObjectGeometryDataPtr_;
  std::vector<RenderObject*>                m_renderObjects_;

  bool m_skipShadowMapGen_       = false;
  bool m_skipUpdateShadowVolume_ = false;
  bool m_isVisible_              = true;
  bool m_dirtyObjectState_       = false;

  bool                                m_isPostUpdate_ = true;
  std::function<void(Object*, float)> m_postUpdateFunc_;

  Object* m_boundBoxObject_    = nullptr;
  Object* m_boundSphereObject_ = nullptr;

  BoundBox    m_boundBox_;
  BoundSphere m_boundSphere_;

  // ======= END: public misc fields   ========================================
};

}  // namespace game_engine

#endif  // GAME_ENGINE_OBJECT_H