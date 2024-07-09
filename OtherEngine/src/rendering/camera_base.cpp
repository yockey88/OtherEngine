/**
 * \file rendering/camera_base.cpp
 **/
#include "rendering/camera_base.hpp"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/geometric.hpp>
#include <SDL.h>

#include "input/mouse.hpp"
#include "input/keyboard.hpp"

namespace other {

  CameraBase::CameraBase(const Ref<CameraBase>& other , CameraProjectionType type)
      : projection_type(type) {
    SetPosition(other->position);
    SetDirection(other->direction);
    SetUp(other->up);
    SetRight(other->right);
    SetWorldUp(other->world_up);

    SetOrientation(other->euler_angles);

    SetViewport(other->viewport_size);
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
    UpdateCoordinateFrame();
    CalculateView();
    CalculateProjection();
    mvp = projection * view;
  }
      
  void CameraBase::UpdateCoordinateFrame() {
    glm::vec3 new_dir; 
    new_dir.x = cos(glm::radians(Yaw())) * cos(glm::radians(Pitch()));
    new_dir.y = sin(glm::radians(Pitch()));
    new_dir.z = sin(glm::radians(Yaw())) * cos(glm::radians(Pitch()));

    SetDirection(glm::normalize(new_dir));
    SetRight(glm::normalize(glm::cross(Direction(), WorldUp())));
    SetUp(glm::normalize(glm::cross(Right(), Direction())));
    SetPosition(Position());
  }

  const glm::mat4& CameraBase::GetMatrix() {
    CalculateMatrix();
    return mvp;     
  }                
                   
  const glm::mat4& CameraBase::ViewMatrix() {
    CalculateMatrix();
    return view;
  }

  const glm::mat4& CameraBase::ProjectionMatrix() {
    CalculateMatrix();
    return projection;
  }

  void CameraBase::MoveForward() { 
    position += speed * direction; 
  }
  
  void CameraBase::MoveBackward() { 
    position -=  speed * direction; 
  }
  
  void CameraBase::MoveLeft() { 
    position -= speed * right;
  }
  
  void CameraBase::MoveRight() { 
    position += speed * right; 
  }
  
  void CameraBase::MoveUp() { 
    position += speed * up; 
  }
  
  void CameraBase::MoveDown() { 
    position -= speed * up;
  }
  
  void CameraBase::Move(const glm::vec3& dir) { 
    position += speed * dir; 
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
    viewport_size = viewport;
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
    view = glm::lookAt(position , position + direction , up);
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
    return viewport_size;
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

  void DefaultUpdateCamera(Ref<CameraBase>& camera) {
    if (Keyboard::Down(Keyboard::Key::OE_W)) {
      camera->MoveForward();
    }
    if (Keyboard::Down(Keyboard::Key::OE_S)) {
      camera->MoveBackward();
    }
    if (Keyboard::Down(Keyboard::Key::OE_A)) {
      camera->MoveLeft();
    }
    if (Keyboard::Down(Keyboard::Key::OE_D)) {
      camera->MoveRight();
    }
    if (Keyboard::Down(Keyboard::Key::OE_SPACE)) {
      camera->MoveUp();
    }
    if (Keyboard::Down(Keyboard::Key::OE_LSHIFT)) {
      camera->MoveDown();
    }

    glm::ivec2 mouse_pos = Mouse::GetPos(); 

    camera->SetLastMouse(camera->Mouse());
    camera->SetMousePos(mouse_pos);
    camera->SetDeltaMouse({ 
      camera->Mouse().x - camera->LastMouse().x ,
      camera->LastMouse().y - camera->Mouse().y
    });

    glm::ivec2 rel_pos = Mouse::GetRelPos();

    camera->SetYaw(camera->Yaw() + (rel_pos.x * camera->Sensitivity()));
    camera->SetPitch(camera->Pitch() - (rel_pos.y * camera->Sensitivity()));

    if (camera->ConstrainPitch()) {
      if (camera->Pitch() > 89.0f) {
        camera->SetPitch(89.0f);
      }

      if (camera->Pitch() < -89.0f) {
        camera->SetPitch(-89.0f);
      }
    }

    camera->UpdateCoordinateFrame();
    camera->CalculateMatrix();
  }

} // namespace other
