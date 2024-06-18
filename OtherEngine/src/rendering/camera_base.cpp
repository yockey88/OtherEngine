/**
 * \file rendering/camera_base.cpp
 **/
#include "rendering/camera_base.hpp"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/geometric.hpp>

namespace other {

  const glm::mat4& CameraBase::GetMatrix() {
    CalculateView();
    CalculateProjection();
    mvp = projection * view;
    return mvp;     
  }                
                   
  const glm::mat4& CameraBase::ViewMatrix() {
    CalculateView();
    mvp = view * projection;
    return view;
  }

  const glm::mat4& CameraBase::ProjectionMatrix() {
    CalculateProjection();
    mvp = view * projection;
    return projection;
  }
  
  void CameraBase::CalculateView() {
    view = glm::lookAt(position , glm::normalize(position) + direction , up);
  }

} // namespace other
