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

  struct Camera;

  struct CameraSnapshotter : public ObjectSerializer<Camera, 2> {
    CameraSnapshotter();

    static size_t Stride() {
      return sizeof(bool) * 2;
    }
  };

  struct Camera : public Component {
    Ref<CameraBase> camera = nullptr;
    bool is_primary = false;
    bool pinned_to_entity_position = false;

    Camera(const Ref<CameraBase>& camera)
        : Component(CAMERA_COMPONENT_INDEX), camera(camera) {}

    ECS_COMPONENT(Camera, CAMERA_COMPONENT_INDEX);
  };

  class CameraSerializer : public ComponentSerializer {
   public:
    COMPONENT_SERIALIZERS(Camera)
  };

}  // namespace other

ECHO_TYPE(
  type(other::Camera, refl::attr::bases<other::Component>),
  field(camera, echo::serializable_field()),
  field(pinned_to_entity_position, echo::serializable_field()),
  field(is_primary)
);

#endif  // !OTHER_ENGINE_CAMERA_HPP
