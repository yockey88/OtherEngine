/**
 * \file rendering/camera_base.cpp
 **/
#include "rendering/camera_base.hpp"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/geometric.hpp>

namespace other {

  CameraBase::CameraBase(const Ref<CameraBase>& other , CameraProjectionType type)
      : projection_type(type) {
    SetPosition(other->position);
    SetDirection(other->direction);
    SetUp(other->up);
    SetRight(other->right);
    SetWorldUp(other->world_up);

    SetOrientation(other->euler_angles);

    SetViewport(other->viewport);
    SetClip(other->clip);

    SetMousePos(other->mouse);
    SetLastMouse(other->last_mouse);
    SetDeltaMouse(other->delta_mouse);

    SetSpeed(other->speed);
    SetSensitivity(other->sensitivity);
    SetFov(other->fov);
    SetZoom(other->zoom);

    SetConstrainPitch(other->constrain_pitch);
    SetPrimary(other->is_primary);
  }

  CameraProjectionType CameraBase::GetCameraProjectionType() const {
    return projection_type;
  }
      
  void CameraBase::CalculateMatrix() {
    CalculateView();
    CalculateProjection();
    mvp = projection * view;
  }

  const glm::mat4& CameraBase::GetMatrix() {
    CalculateMatrix();
    return mvp;     
  }                
                   
  const glm::mat4& CameraBase::ViewMatrix() {
    CalculateView();
    mvp = projection * view;
    return view;
  }

  const glm::mat4& CameraBase::ProjectionMatrix() {
    CalculateProjection();
    mvp = projection * view;
    return projection;
  }

  void CameraBase::MoveForward(float dt) { 
    position += direction * speed * dt; 
  }
  
  void CameraBase::MoveBackward(float dt) { 
    position -= direction * speed * dt; 
  }
  
  void CameraBase::MoveLeft(float dt) { 
    position -= right * speed * dt; 
  }
  
  void CameraBase::MoveRight(float dt) { 
    position += right * speed * dt; 
  }
  
  void CameraBase::MoveUp(float dt) { 
    position += up * speed * dt; 
  }
  
  void CameraBase::MoveDown(float dt) { 
    position -= up * speed * dt; 
  }
  
  void CameraBase::Move(const glm::vec3& direction , float dt) { 
    position += direction * speed * dt; 
  }

  void CameraBase::SetPosition(const glm::vec3& position) { 
    this->position = position; 
  }
  
  void CameraBase::SetDirection(const glm::vec3& direction) { 
    this->direction = direction; 
  }
  
  void CameraBase::SetUp(const glm::vec3& up) { 
    this->up = up; 
  }
  
  void CameraBase::SetRight(const glm::vec3& right) { 
    this->right = right; 
  }
  
  void CameraBase::SetWorldUp(const glm::vec3& world_up) { 
    this->world_up = world_up; 
  }

  void CameraBase::SetOrientation(const glm::vec3& orientation) { 
    this->euler_angles = orientation; 
  }
  
  void CameraBase::SetYaw(float yaw) { 
    this->euler_angles.x = yaw; 
  }
  
  void CameraBase::SetPitch(float pitch) { 
    this->euler_angles.y = pitch; 
  }
  
  void CameraBase::SetRoll(float roll) { 
    this->euler_angles.z = roll; 
  }

  void CameraBase::SetViewport(const glm::ivec2& viewport) { 
    this->viewport = viewport; 
  }
  
  void CameraBase::SetClip(const glm::vec2& clip) { 
    this->clip = clip; 
  }
  
  void CameraBase::SetMousePos(const glm::vec2& mouse) { 
    this->mouse = mouse; 
  }
  
  void CameraBase::SetLastMouse(const glm::vec2& last_mouse) { 
    this->last_mouse = last_mouse; 
  }
  
  void CameraBase::SetDeltaMouse(const glm::vec2& delta_mouse) { 
    this->delta_mouse = delta_mouse; 
  }

  void CameraBase::SetSpeed(float speed) { 
    this->speed = speed; 
  }
  
  void CameraBase::SetSensitivity(float sensitivity) { 
    this->sensitivity = sensitivity; 
  }
  
  void CameraBase::SetFov(float fov) { 
    this->fov = fov; 
  }
  
  void CameraBase::SetZoom(float zoom) { 
    this->zoom = zoom; 
  }

  void CameraBase::SetConstrainPitch(bool constrain_pitch) { 
    this->constrain_pitch = constrain_pitch; 
  }
  
  void CameraBase::SetPrimary(bool primary) { 
    this->is_primary = primary; 
  }
  
  void CameraBase::CalculateView() {
    view = glm::lookAt(position , glm::normalize(position) + direction , up);
  }

  const glm::vec3& CameraBase::Position() const {
    return position;
  }

  const glm::vec3& CameraBase::Direction() const {
    return direction;
  }

  const glm::vec3& CameraBase::Up() const {
    return right;
  }

  const glm::vec3& CameraBase::Right() const {
    return right;
  }

  const glm::vec3& CameraBase::WorldUp() const {
    return world_up;
  } 

  const glm::vec3& CameraBase::Orientation() const {
    return euler_angles;
  }

  float CameraBase::Yaw() const {
    return euler_angles.x;
  }

  float CameraBase::Pitch() const {
    return euler_angles.y;
  }

  float CameraBase::Roll() const {
    return euler_angles.z;
  }

  const glm::ivec2& CameraBase::Viewport() const {
    return viewport;
  }

  const glm::vec2& CameraBase::Clip() const {
    return clip;
  }

  const glm::vec2& CameraBase::Mouse() const {
    return mouse;
  }

  const glm::vec2& CameraBase::LastMouse() const {
    return last_mouse;
  }

  const glm::vec2& CameraBase::DeltaMouse() const {
    return delta_mouse;
  }

  float CameraBase::Speed() const {
    return speed;
  }

  float CameraBase::Sensitivity() const {
    return sensitivity;
  }

  float CameraBase::FOV() const {
    return fov;
  }
  
  float CameraBase::Zoom() const {
    return zoom;
  }

  bool CameraBase::ConstrainPitch() const {
    return constrain_pitch;
  }

  bool CameraBase::IsPrimary() const {
    return is_primary;
  }

} // namespace other
