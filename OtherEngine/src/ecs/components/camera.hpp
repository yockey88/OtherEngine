/**
 * \file ecs/components/camera.hpp
 **/
#ifndef OTHER_ENGINE_CAMERA_HPP
#define OTHER_ENGINE_CAMERA_HPP

#include "core/ref.hpp"
#include "ecs/component.hpp"
#include "ecs/component_serializer.hpp"
#include "rendering/camera_base.hpp"

namespace other {

  struct Camera : public Component {
    Ref<CameraBase> camera = nullptr;
    bool pinned_to_entity_position = true;
    bool is_primary = false;
    
    Camera(const Ref<CameraBase>& camera)
      : Component(kCameraIndex) , camera(camera) {}

    ECS_COMPONENT(Camera , kCameraIndex);
  };

  class CameraSerializer : public ComponentSerializer {
    public:
      COMPONENT_SERIALIZERS(Camera)
  };

} // namespace other

#endif // !OTHER_ENGINE_CAMERA_HPP
