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
        : Component(CAMERA_COMPONENT_INDEX), camera(camera) {}

    ECS_COMPONENT(Camera, CAMERA_COMPONENT_INDEX);
  };

  class CameraSerializer : public ComponentSerializer {
   public:
    COMPONENT_SERIALIZERS(Camera)
  };

  // struct CameraSnapshotter : public ObjectSerializer<Camera, 3> {
  //   CameraSnapshotter() {
  //     AddField<Ref<CameraBase>, 0>(&Camera::camera);
  //     AddField<bool, 1>(&Camera::pinned_to_entity_position);
  //     AddField<bool, 2>(&Camera::is_primary);
  //   }
  // };

}  // namespace other

ECHO_TYPE(
  type(other::Camera, refl::attr::bases<other::Component>),
  field(camera),
  field(pinned_to_entity_position),
  field(is_primary)
);

#endif  // !OTHER_ENGINE_CAMERA_HPP
