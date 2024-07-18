/**
 * \file rendering/perspective_camera.cpp
 **/
#include "rendering/perspective_camera.hpp"

#include <glm/ext/matrix_clip_space.hpp>

namespace other {
  
  std::string PerspectiveCamera::GetCameraTypeString() const {
    return "perspective";
  }

  void PerspectiveCamera::CalculateProjection() {
    projection = glm::perspective(glm::radians(fov) , static_cast<float>(viewport_size.x) / viewport_size.y , clip.x , clip.y);
  }

} // namespace other
