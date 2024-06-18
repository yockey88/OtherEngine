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

      const glm::mat4& GetMatrix();
      const glm::mat4& ViewMatrix();
      const glm::mat4& ProjectionMatrix();

      inline void MoveForward(float dt) { position += direction * speed * dt; }
      inline void MoveBackward(float dt) { position -= direction * speed * dt; }
      inline void MoveLeft(float dt) { position -= right * speed * dt; }
      inline void MoveRight(float dt) { position += right * speed * dt; }
      inline void MoveUp(float dt) { position += up * speed * dt; }
      inline void MoveDown(float dt) { position -= up * speed * dt; }
      inline void Move(const glm::vec3& direction , float dt) { position += direction * speed * dt; }

      inline void SetPosition(const glm::vec3& position) { this->position = position; }
      inline void SetDirection(const glm::vec3& direction) { this->direction = direction; }
      inline void SetUp(const glm::vec3& up) { this->up = up; }
      inline void SetRight(const glm::vec3& right) { this->right = right; }
      inline void SetWorldUp(const glm::vec3& world_up) { this->world_up = world_up; }

      inline void SetOrientation(const glm::vec3& orientation) { this->euler_angles = orientation; }
      inline void SetYaw(float yaw) { this->euler_angles.x = yaw; }
      inline void SetPitch(float pitch) { this->euler_angles.y = pitch; }
      inline void SetRoll(float roll) { this->euler_angles.z = roll; }

      inline void SetViewport(const glm::ivec2& viewport) { this->viewport = viewport; }
      inline void SetClip(const glm::vec2& clip) { this->clip = clip; }
      inline void SetMousePos(const glm::vec2& mouse) { this->mouse = mouse; }
      inline void SetLastMouse(const glm::vec2& last_mouse) { this->last_mouse = last_mouse; }
      inline void SetDeltaMouse(const glm::vec2& delta_mouse) { this->delta_mouse = delta_mouse; }

      inline void SetSpeed(float speed) { this->speed = speed; }
      inline void SetSensitivity(float sensitivity) { this->sensitivity = sensitivity; }
      inline void SetFov(float fov) { this->fov = fov; }
      inline void SetZoom(float zoom) { this->zoom = zoom; }

      inline void SetConstrainPitch(bool constrain_pitch) { this->constrain_pitch = constrain_pitch; }
      inline void SetPrimary(bool primary) { this->is_primary = primary; }

      inline const glm::vec3& Position() const { return position; }
      inline const glm::vec3& Direction() const { return direction; }
      inline const glm::vec3& Up() const { return up; }
      inline const glm::vec3& Right() const { return right; }
      inline const glm::vec3& WorldUp() const { return world_up; }

      inline const glm::vec3& Orientation() const { return euler_angles; }
      inline float Yaw() const { return euler_angles.x; }
      inline float Pitch() const { return euler_angles.y; }
      inline float Roll() const { return euler_angles.z; }

      inline const glm::ivec2& Viewport() const { return viewport; }
      inline const glm::vec2& Clip() const { return clip; }
      inline const glm::vec2& Mouse() const { return mouse; }
      inline const glm::vec2& LastMouse() const { return last_mouse; }
      inline const glm::vec2& DeltaMouse() const { return delta_mouse; }

      inline float Speed() const { return speed; }
      inline float Sensitivity() const { return sensitivity; }
      inline float FOV() const { return fov; }
      inline float Zoom() const { return zoom; }

      inline bool ConstrainPitch() const { return constrain_pitch; }

      virtual void Rotate(const glm::quat& quat) = 0;
      virtual void RotateAround(const glm::vec3& v , float degrees) = 0;

    protected:
      glm::vec3 position = glm::vec3(0.0f , 0.0f , 3.0f);
      glm::vec3 direction = glm::vec3(0.0f , 0.0f , -1.0f);
      glm::vec3 up = glm::vec3(0.0f , 1.0f , 0.0f);
      glm::vec3 right = glm::vec3(1.0f , 0.0f , 0.0f);
      glm::vec3 world_up = glm::vec3(0.0f , 1.0f , 0.0f);

      // roll , pitch , yaw 
      glm::vec3 euler_angles = glm::vec3(-90.0f , 0.0f , 0.0f);
      glm::ivec2 viewport = glm::ivec2(800 , 600);

      // near clip , far clip
      glm::vec2 clip = glm::vec2(0.1f , 100.0f);
      glm::vec2 mouse = glm::vec2(0.0f , 0.0f);
      glm::vec2 last_mouse = glm::vec2(0.0f , 0.0f);
      glm::vec2 delta_mouse = glm::vec2(0.0f , 0.0f);

      float speed = 0.1f;
      float sensitivity = 0.2f;
      float fov = 75.0f;
      float zoom = 45.0f;

      bool constrain_pitch = true;
      bool is_primary = false;

      glm::mat4 view = glm::mat4(1.f);
      glm::mat4 projection = glm::mat4(1.f);

    private:
      glm::mat4 mvp = glm::mat4(1.f);

      void CalculateView(); 
      virtual void CalculateProjection() = 0;
  };

} // namespace other

#endif // !OTHER_ENGINE_CAMERA_BASE_HPP
