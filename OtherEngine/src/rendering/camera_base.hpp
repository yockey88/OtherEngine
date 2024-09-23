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
      CameraBase(CameraProjectionType type , const glm::ivec2& viewport_size) 
          : viewport_size(viewport_size) , projection_type(type) {
      }
      
      /// This is NOT a copy constructor because the type is Ref<CameraBase>
      CameraBase(const Ref<CameraBase>& other , CameraProjectionType type);

      virtual ~CameraBase() {}

      CameraProjectionType GetCameraProjectionType() const;

      void CalculateMatrix();
      void UpdateCoordinateFrame();

      const glm::mat4& GetMatrix() const;
      const glm::mat4& ViewMatrix() const;
      const glm::mat4& ProjectionMatrix() const;

      void MoveForward();
      void MoveBackward();
      void MoveLeft();
      void MoveRight();
      void MoveUp();
      void MoveDown();
      void Move(const glm::vec3& direction);

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

      void Rotate(const glm::quat& quat)  {}
      void RotateAround(const glm::vec3& v , float degrees) {}

      virtual std::string GetCameraTypeString() const = 0;

    protected:
      friend bool DrawCamera(Entity* ent);

      glm::ivec2 viewport_size = { 800 , 600 };

      glm::vec3 position{ 0.f };
      glm::vec3 direction{ 0.0f , 0.0f , -1.0f };
      glm::vec3 up{ 0.0f , 1.0f , 0.0f };
      glm::vec3 right{ 1.0f , 0.0f , 0.0f };
      glm::vec3 world_up{ 0.0f , 1.0f , 0.0f };

      // roll , pitch , yaw 
      glm::vec3 euler_angles{ -90.0f , 0.0f , 0.0f };

      // near clip , far clip
      glm::vec2 clip{ 0.1f , 10000.0f };
      glm::vec2 mouse{ 0.0f , 0.0f };
      glm::vec2 last_mouse{ 0.0f , 0.0f };
      glm::vec2 delta_mouse{ 0.0f , 0.0f };

      float speed = 0.1f;
      float sensitivity = 0.2f;
      float fov = 75.0f;
      float zoom = 45.0f;

      bool constrain_pitch = true;

      glm::mat4 view = glm::mat4(1.f);
      glm::mat4 projection = glm::mat4(1.f);

    private:
      CameraProjectionType projection_type;
      glm::mat4 mvp = glm::mat4(1.f);

      void CalculateView(); 
      virtual void CalculateProjection() = 0;
  };

  void DefaultUpdateCamera(Ref<CameraBase>& camera); 

} // namespace other

#endif // !OTHER_ENGINE_CAMERA_BASE_HPP
