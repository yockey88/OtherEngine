/**
 * \file ecs/components/camera.hpp
 **/
#ifndef OTHER_ENGINE_CAMERA_HPP
#define OTHER_ENGINE_CAMERA_HPP

#include "ecs/component.hpp"
#include "ecs/component_serializer.hpp"

namespace other {

  struct Camera : public Component {
    ECS_COMPONENT(Camera , kCameraIndex);
  };

  class CameraSerializer : public ComponentSerializer {
    public:
      COMPONENT_SERIALIZERS(Camera)
  };

} // namespace other

#endif // !OTHER_ENGINE_CAMERA_HPP
