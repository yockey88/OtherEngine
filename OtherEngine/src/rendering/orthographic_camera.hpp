/**
 * \file rendering/orthographic_camera.hpp
 **/
#ifndef OTHER_ENGINE_ORTHOGRAPHIC_CAMERA_HPP
#define OTHER_ENGINE_ORTHOGRAPHIC_CAMERA_HPP

#include "rendering/camera_base.hpp"

namespace other {

  class OrthographicCamera : public CameraBase {
    public:
      virtual void Rotate(const glm::quat& quat) override {}
      virtual void RotateAround(const glm::vec3& v , float degrees) override {}

    private:
      virtual void CalculateProjection() override;
  };

} // namespace other

#endif // !OTHER_ENGINE_ORTHOGRAPHIC_CAMERA_HPP
