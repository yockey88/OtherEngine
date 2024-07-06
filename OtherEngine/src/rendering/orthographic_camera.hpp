/**
 * \file rendering/orthographic_camera.hpp
 **/
#ifndef OTHER_ENGINE_ORTHOGRAPHIC_CAMERA_HPP
#define OTHER_ENGINE_ORTHOGRAPHIC_CAMERA_HPP

#include "rendering/camera_base.hpp"

namespace other {

  class OrthographicCamera : public CameraBase {
    public:
      OrthographicCamera(const glm::ivec2& viewport_size)
          : CameraBase(CameraProjectionType::ORTHOGRAPHIC , viewport_size) {}
      
      OrthographicCamera(const Ref<CameraBase>& other) 
          : CameraBase(other , CameraProjectionType::ORTHOGRAPHIC) {}

      virtual std::string GetCameraTypeString() const override;

    private:
      virtual void CalculateProjection() override;
  };

} // namespace other

#endif // !OTHER_ENGINE_ORTHOGRAPHIC_CAMERA_HPP
