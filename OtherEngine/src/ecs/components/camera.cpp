/**
 * \file ecs/components/camera.cpp
 **/
#include "ecs/components/camera.hpp"

#include "core/logger.hpp"
#include "scene/scene.hpp"

namespace other {

  void CameraSerializer::Serialize(std::ostream& stream , Entity* entity , const Ref<Scene>& scene) const {
  }

  void CameraSerializer::Deserialize(Entity* entity , const ConfigTable& scene_table , Ref<Scene>& scene) const {
    OE_ASSERT(entity != nullptr && scene != nullptr , "Attempting to deserialize a camera into null entity or scene");
    
  }

} // namespace other
