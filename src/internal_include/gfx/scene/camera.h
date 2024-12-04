// TODO:
// - move to more appropriate folder (e.g. scene)
// - consider moving scene folder to a more appropriate location (gfx is not the
// very best place for it)

#ifndef GAME_ENGINE_CAMERA_H
#define GAME_ENGINE_CAMERA_H

#include "config/config_manager.h"
#include "gfx/rhi/feature_switch.h"
#include "utils/hot_reload/hot_reload_manager.h"
#include "utils/math/math_util.h"
#include "utils/math/plane.h"
#include "utils/path_manager/path_manager.h"
#include "utils/service/service_locator.h"

#include <math_library/graphics.h>
#include <math_library/matrix.h>
#include <math_library/quaternion.h>
#include <math_library/vector.h>

#include <array>
#include <map>
#include <memory>

// defines from windows.h
#undef near
#undef far

namespace game_engine {

enum class ECameraType {
  Perspective = 0,
  Orthographic,
  MAX
};

struct CameraParameters {
  math::Vector3Df   m_position;
  math::Vector3Df   m_direction;
  math::Quaternionf m_orientation;
  float             m_fov;
  float             m_near;
  float             m_far;
  // TODO: make uint32_t and math::Dimension2ui
  float             m_width;
  float             m_height;
  ECameraType       m_type;

  // bool            m_isPerspectiveProjection;

  static CameraParameters s_fromConfig(const ConfigValue& value);
};

// TODO:
// - add frustrum rect for triangle for debug / visualization purposes
// - consider add cameraParameters member instead of bunch of parameters
// (consider implications of this aproach)
// - consider adding update feature only when camera parameters change
class Camera {
  public:
  // ======= BEGIN: public constructors =======================================

  // Camera()
  //     : m_type_(ECameraType::Perspective) {}

  Camera(const math::Vector3Df&   pos,
         const math::Vector3Df&   direction,
         const math::Quaternionf& orientation,
         const math::Vector3Df&   up,
         float                    fov,
         float                    near,
         float                    far,
         float                    width,
         float                    height,
         ECameraType              type = ECameraType::Perspective)
      : m_type_(type)
      , m_position_(pos)
      , m_direction_(direction)
      , m_orientation_(orientation)
      , m_worldUp_(up)
      , m_fov_(fov)
      , m_near_(near)
      , m_far_(far)
      , m_width_(static_cast<int32_t>(width))
      , m_height_(static_cast<int32_t>(height)) {
    updateCamera();
  }

  // ======= END: public constructors =========================================

  // ======= BEGIN: public destructor =========================================

  virtual ~Camera() {}

  // ======= END: public destructor ===========================================

  // ======= BEGIN: public getters ============================================

  // TODO: remove Vector postfix
  math::Vector3Df getForward() const {
    return m_view_.getColumn<2>().resizedCopy<3>();
  }

  math::Vector3Df getUp() const {
    return m_view_.getColumn<1>().resizedCopy<3>();
  }

  math::Vector3Df getRight() const {
    return m_view_.getColumn<0>().resizedCopy<3>();
  }

  const math::Matrix4f<>& getViewProjection() const {
    return m_viewProjection_;
  }

  const math::Matrix4f<>& getInverseViewProjection() const {
    return m_invViewProjection_;
  }

  const math::Vector3Df& getPosition() const { return m_position_; }

  const math::Quaternionf& getOrientation() const { return m_orientation_; }

  // ======= END: public getters ==============================================

  // ======= BEGIN: public setters ============================================

  auto setPosition(const math::Vector3Df& pos) { m_position_ = pos; }

  auto setOrientation(const math::Quaternionf& orientation) {
    m_orientation_ = orientation;
  }

  // ======= END: public setters   ============================================

  // ======= BEGIN: public misc methods =======================================

  virtual math::Matrix4f<> createView() const {
    // TODO: consider adding RH / LH selection logic
    auto rotationMatrix = m_orientation_.toRotationMatrix();
    // TODO: consider using another method to get forward and up vectors
    // (like q * v * q^-1)
    auto forward = rotationMatrix.getRow<2>();
    auto up      = rotationMatrix.getRow<1>();
    return math::g_lookToLh(m_position_, forward, up);
  }

  virtual math::Matrix4f<> createProjection() const {
    const auto kRatio
        = static_cast<float>(m_width_) / static_cast<float>(m_height_);
    if (m_type_ == ECameraType::Perspective) {
      if (m_isInfinityFar_) {
        return math::g_perspectiveLhZoInf(m_fov_, kRatio, m_near_);
      }
      return math::g_perspectiveLhZo(m_fov_, kRatio, m_near_, m_far_);
    }
    return math::g_orthoLhZo(
        (float)m_width_, (float)m_height_, m_near_, m_far_);
  }

  void updateCamera() {
    m_view_       = createView();
    m_projection_ = createProjection();
    //  TODO: if this is row major, then the order should be reversed
    m_viewProjection_    = m_view_ * m_projection_;
    m_invViewProjection_ = m_viewProjection_.inverse();
    if (m_type_ == ECameraType::Perspective) {
#if USE_REVERSEZ_PERSPECTIVE_SHADOW
      // TODO: add implementation
#else
      m_reverseZProjection_ = Projection;
#endif
    }
  }

  void updateFromConfig() {
    auto configManager = ServiceLocator::s_get<ConfigManager>();
    auto config        = configManager
                      ->getConfig(PathManager::s_getDebugPath() / "config.json")
                      .lock();

    if (config) {
      m_worldUp_ = config->get<math::Vector3Df>("worldUp");

      auto cameraParameters = config->get<CameraParameters>("camera");
      m_position_           = cameraParameters.m_position;
      m_direction_          = cameraParameters.m_direction;
      m_orientation_        = cameraParameters.m_orientation;
      m_fov_                = cameraParameters.m_fov;
      m_near_               = cameraParameters.m_near;
      m_far_                = cameraParameters.m_far;
      m_type_               = cameraParameters.m_type;
    }
  }

  // ======= END: public misc methods =========================================

  // ======= BEGIN: public misc fields ========================================

  ECameraType m_type_;

  math::Vector3Df m_position_;
  math::Vector3Df m_direction_;
  math::Vector3Df m_worldUp_;

  math::Quaternionf m_orientation_;

  math::Matrix4f<> m_view_;
  math::Matrix4f<> m_projection_;
  math::Matrix4f<> m_viewProjection_;
  math::Matrix4f<> m_invViewProjection_;
  math::Matrix4f<> m_reverseZProjection_;
  // bool             m_isPerspectiveProjection_ = true;
  bool             m_isInfinityFar_ = false;

  float m_fov_  = 0.0f;  // in radians
  float m_near_ = 0.0f;
  float m_far_  = 0.0f;

  int32_t m_width_  = 0;
  int32_t m_height_ = 0;

  // ======= END: public misc fields ==========================================
};

}  // namespace game_engine

// define back macros from windows.h
#define near
#define far

#endif  // GAME_ENGINE_CAMERA_H
