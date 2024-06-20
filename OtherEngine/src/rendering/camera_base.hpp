/**
 * \file rendering/camera_base.hpp
 **/
#ifndef OTHER_ENGINE_CAMERA_BASE_HPP
#define OTHER_ENGINE_CAMERA_BASE_HPP

#include <string>

#include <glm/glm.hpp>

#include "core/ref.hpp"
#include "core/ref_counted.hpp"

namespace other {

  class Entity;

  enum CameraProjectionType : uint32_t {
    PERSPECTIVE  = 0 ,
    ORTHOGRAPHIC = 1 ,

    INVALID_CAMERA_PROJ ,
    NUM_CAMERA_PROJ = INVALID_CAMERA_PROJ ,
  };

  class CameraBase : public RefCounted {
    public:
      CameraBase(CameraProjectionType type) 
        : projection_type(type) {}
      
      /// This is NOT a copy constructor because the type is Ref<CameraBase>
      CameraBase(const Ref<CameraBase>& other , CameraProjectionType type);

      virtual ~CameraBase() {}

      CameraProjectionType GetCameraProjectionType() const;

      void CalculateMatrix();

      const glm::mat4& GetMatrix();
      const glm::mat4& ViewMatrix();
      const glm::mat4& ProjectionMatrix();

      void MoveForward(float dt);
      void MoveBackward(float dt);
      void MoveLeft(float dt);
      void MoveRight(float dt);
      void MoveUp(float dt);
      void MoveDown(float dt);
      void Move(const glm::vec3& direction , float dt);

      void SetPosition(const glm::vec3& position);
      void SetDirection(const glm::vec3& direction);
      void SetUp(const glm::vec3& up);
      void SetRight(const glm::vec3& right);
      void SetWorldUp(const glm::vec3& world_up);

      void SetOrientation(const glm::vec3& orientation);
      void SetYaw(float yaw);
      void SetPitch(float pitch);
      void SetRoll(float roll);

      void SetViewport(const glm::ivec2& viewport);
      void SetClip(const glm::vec2& clip);
      void SetMousePos(const glm::vec2& mouse);
      void SetLastMouse(const glm::vec2& last_mouse);
      void SetDeltaMouse(const glm::vec2& delta_mouse);

      void SetSpeed(float speed);
      void SetSensitivity(float sensitivity);
      void SetFov(float fov);
      void SetZoom(float zoom);

      void SetConstrainPitch(bool constrain_pitch);
      void SetPrimary(bool primary);

      const glm::vec3& Position() const;
      const glm::vec3& Direction() const;
      const glm::vec3& Up() const;
      const glm::vec3& Right() const;
      const glm::vec3& WorldUp() const; 

      const glm::vec3& Orientation() const;
      float Yaw() const;
      float Pitch() const;
      float Roll() const;

      const glm::ivec2& Viewport() const;
      const glm::vec2& Clip() const;
      const glm::vec2& Mouse() const;
      const glm::vec2& LastMouse() const;
      const glm::vec2& DeltaMouse() const;

      float Speed() const;
      float Sensitivity() const;
      float FOV() const;
      float Zoom() const;

      bool ConstrainPitch() const;
      bool IsPrimary() const;

      void Rotate(const glm::quat& quat)  {}
      void RotateAround(const glm::vec3& v , float degrees) {}

      virtual std::string GetCameraTypeString() const = 0;

    protected:
      friend void DrawCamera(Entity* ent);

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
      CameraProjectionType projection_type;
      glm::mat4 mvp = glm::mat4(1.f);

      void CalculateView(); 
      virtual void CalculateProjection() = 0;
  };

} // namespace other

#endif // !OTHER_ENGINE_CAMERA_BASE_HPP
