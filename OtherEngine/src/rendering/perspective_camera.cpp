/**
 * \file rendering/perspective_camera.cpp
 **/
#include "rendering/perspective_camera.hpp"

#include <glm/ext/matrix_clip_space.hpp>

#include "rendering/renderer.hpp"

namespace other {
  
  std::string PerspectiveCamera::GetCameraTypeString() const {
    return "perspective";
  }

  void PerspectiveCamera::CalculateProjection() {
    glm::ivec2 window_size = Renderer::WindowSize();
    projection = glm::perspective(glm::radians(fov) , static_cast<float>(window_size.x) / window_size.y , 
                                  clip.x , clip.y);
  }

} // namespace other
