/**
 * \file rendering/orthographic_camera.cpp
 **/
#include "rendering/orthographic_camera.hpp"

namespace other {

  std::string OrthographicCamera::GetCameraTypeString() const {
    return "orthographic";
  }

  void OrthographicCamera::CalculateProjection() {}

} // namespace other
