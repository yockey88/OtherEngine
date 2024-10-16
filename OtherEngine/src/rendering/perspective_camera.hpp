/**
 * \file rendering/perspective_camera.hpp
 **/
#ifndef OTHER_ENGINE_PERSPECTIVE_CAMERA_HPP
#define OTHER_ENGINE_PERSPECTIVE_CAMERA_HPP

#include "rendering/camera_base.hpp"

namespace other {

  class PerspectiveCamera : public CameraBase {
    public:
      PerspectiveCamera(const glm::ivec2& viewport_size)
          : CameraBase(CameraProjectionType::PERSPECTIVE , viewport_size) {}
      
      PerspectiveCamera(const Ref<CameraBase>& other)
          : CameraBase(other , CameraProjectionType::PERSPECTIVE) {}

      virtual std::string GetCameraTypeString() const override;

    private:
      virtual void CalculateProjection() override;
  };

} // namespace other

#endif // !OTHER_ENGINE_PERSPECTIVE_CAMERA_HPP
