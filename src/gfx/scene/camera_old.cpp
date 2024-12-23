// TODO:
// - move to more appropriate folder (e.g. scene)
// - consider moving scene folder to a more appropriate location (gfx is not the
// very best place for it)

#include "gfx/scene/camera_old.h"

namespace game_engine {

CameraParametersDeprecated CameraParametersDeprecated::s_fromConfig(
    const ConfigValue& value) {
  CameraParametersDeprecated params;
  // TODO: make conversion for math::Vector3Df

  // auto configManager = ServiceLocator::s_get<ConfigManager>();
  // auto config
  //     = configManager->getConfig(PathManager::s_getDebugPath() /
  //     "config.json")
  //           .lock();

  auto pos = value["pos"].GetArray();

  for (size_t i = 0; i < pos.Size(); i++) {
    params.m_position.coeffRef(i) = pos[i].GetFloat();
  }

  auto direction = value["direction"].GetArray();

  for (size_t i = 0; i < direction.Size(); i++) {
    params.m_direction.coeffRef(i) = direction[i].GetFloat();
  }

  auto orientation = value["orientation"].GetArray();
  params.m_orientation.setX(orientation[0].GetFloat());
  params.m_orientation.setY(orientation[1].GetFloat());
  params.m_orientation.setZ(orientation[2].GetFloat());
  params.m_orientation.setW(orientation[3].GetFloat());

  params.m_fov  = value["fov"].GetFloat();
  params.m_near = value["near"].GetFloat();
  params.m_far  = value["far"].GetFloat();
  params.m_type = static_cast<ECameraTypeDeprecated>(value["type"].GetInt());

  return params;
}

}  // namespace game_engine
