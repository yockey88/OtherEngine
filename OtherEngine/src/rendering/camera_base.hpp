/**
 * \file rendering/camera_base.hpp
 **/
#ifndef OTHER_ENGINE_CAMERA_BASE_HPP
#define OTHER_ENGINE_CAMERA_BASE_HPP

#include <glm/glm.hpp>

#include "core/ref_counted.hpp"

namespace other {

  class CameraBase : public RefCounted {
    public:
      CameraBase() {}
      virtual ~CameraBase() {}

      const glm::mat4& ViewMatrix();
      const glm::mat4& ProjectionMatrix();

      virtual void Rotate(const glm::quat& quat) = 0;
      virtual void RotateAround(const glm::vec3& v , float degrees) = 0;

    private:
      glm::mat4 view;
      glm::mat4 projection;

      virtual void CalculateView() = 0;
      virtual void CalculateProjection() = 0;
  };

} // namespace other

#endif // !OTHER_ENGINE_CAMERA_BASE_HPP
