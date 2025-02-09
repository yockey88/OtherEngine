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

  struct CameraSnapshotter : public ObjectSerializer<Camera, 4> {
    CameraSnapshotter();

    static size_t Stride() {
      return sizeof(bool) + sizeof(glm::vec3) * 3;
    }
  };

  struct Camera : public Component {
    Ref<CameraBase> camera = nullptr;
    bool is_primary = false;

    /// have to store these here too so cameras can be reset correctly from a state capture
    glm::vec3 camera_position = glm::vec3(0.f);
    glm::vec3 camera_direction = glm::vec3(0.f);
    glm::vec3 camera_up = glm::vec3(0.f);

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
  field(camera),
  field(is_primary, echo::serializable_field()),
  field(camera_position, echo::serializable_field()),
  field(camera_direction, echo::serializable_field()),
  field(camera_up, echo::serializable_field())
);

#endif  // !OTHER_ENGINE_CAMERA_HPP
