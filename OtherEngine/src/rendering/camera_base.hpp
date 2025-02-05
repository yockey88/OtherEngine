/**
 * \file rendering/camera_base.hpp
 **/
#ifndef OTHER_ENGINE_CAMERA_BASE_HPP
#define OTHER_ENGINE_CAMERA_BASE_HPP

#include <string>

#include <glm/glm.hpp>
#include <reflection/echo_defines.hpp>

#include "core/ref.hpp"
#include "core/ref_counted.hpp"

namespace other {

  class Entity;

  enum CameraProjectionType : uint32_t {
    PERSPECTIVE = 0,
    ORTHOGRAPHIC = 1,

    INVALID_CAMERA_PROJ,
    NUM_CAMERA_PROJ = INVALID_CAMERA_PROJ,
  };

  class CameraBase : public RefCounted {
   public:
    CameraBase(CameraProjectionType type, const glm::ivec2& viewport_size)
        : viewport_size(viewport_size), projection_type(type) {
    }

    /// This is NOT a copy constructor because the type is Ref<CameraBase>
    CameraBase(const Ref<CameraBase>& other, CameraProjectionType type);

    virtual ~CameraBase() {}

    CameraProjectionType GetCameraProjectionType() const;

    void CalculateMatrix();
    void UpdateCoordinateFrame();

    const glm::mat4& GetMatrix();
    const glm::mat4& InverseMatrix();
    const glm::mat4& ViewMatrix();
    const glm::mat4& ProjectionMatrix();

    void MoveForward();
    void MoveBackward();
    void MoveLeft();
    void MoveRight();
    void MoveUp();
    void MoveDown();
    void Move(const glm::vec3& direction);

    void SetPosition(const glm::vec3& position);
    void SetDirection(const glm::vec3& direction);
    void SetTarget(const glm::vec3& target);
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
    const glm::vec3& Target() const;
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

    void Rotate(const glm::quat& quat) {}
    void RotateAround(const glm::vec3& v, float degrees) {}

    virtual std::string GetCameraTypeString() const = 0;

    bool locked = false;

   protected:
    friend bool DrawCamera(Entity* ent);

    glm::ivec2 viewport_size = { 800, 600 };

    glm::vec3 position{ 0.f };
    glm::vec3 direction{ 0.0f, 0.0f, -1.0f };
    glm::vec3 target{ 0.0f, 0.0f, 0.0f };
    glm::vec3 up{ 0.0f, 1.0f, 0.0f };
    glm::vec3 right{ 1.0f, 0.0f, 0.0f };
    glm::vec3 world_up{ 0.0f, 1.0f, 0.0f };

    // roll , pitch , yaw
    glm::vec3 euler_angles{ -90.0f, 0.0f, 0.0f };

    // near clip , far clip
    glm::vec2 clip{ 0.1f, 10000.0f };
    glm::vec2 mouse{ 0.0f, 0.0f };
    glm::vec2 last_mouse{ 0.0f, 0.0f };
    glm::vec2 delta_mouse{ 0.0f, 0.0f };

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
    glm::mat4 inverse_mvp = glm::mat4(1.f);

    void CalculateView();
    virtual void CalculateProjection() = 0;
  };

  void DefaultUpdateCamera(Ref<CameraBase>& camera);

}  // namespace other

ECHO_TYPE(
  type(other::CameraBase),
  func(GetCameraProjectionType),
  func(CalculateMatrix),
  func(UpdateCoordinateFrame),
  func(GetMatrix),
  func(ViewMatrix),
  func(ProjectionMatrix),
  func(MoveForward),
  func(MoveBackward),
  func(MoveLeft),
  func(MoveRight),
  func(MoveUp),
  func(MoveDown),
  func(Move),
  func(SetPosition),
  func(SetDirection),
  func(SetUp),
  func(SetRight),
  func(SetWorldUp),
  func(SetOrientation),
  func(SetYaw),
  func(SetPitch),
  func(SetRoll),
  func(SetViewport),
  func(SetClip),
  func(SetMousePos),
  func(SetLastMouse),
  func(SetDeltaMouse),
  func(SetSpeed),
  func(SetSensitivity),
  func(SetFov),
  func(SetZoom),
  func(SetConstrainPitch),
  func(Position),
  func(Direction),
  func(Up),
  func(Right),
  func(WorldUp),
  func(Orientation),
  func(Yaw),
  func(Pitch),
  func(Roll),
  func(Viewport),
  func(Clip),
  func(Mouse),
  func(LastMouse),
  func(DeltaMouse),
  func(Speed),
  func(Sensitivity),
  func(FOV),
  func(Zoom),
  func(ConstrainPitch),
  func(Rotate),
  func(RotateAround),
  func(GetCameraTypeString)
);

#endif  // !OTHER_ENGINE_CAMERA_BASE_HPP
