using System;
using System.Runtime.CompilerServices;

namespace enginey {

  public static class Engine {
#region Math
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern void Mat4LookAt(ref Vec3 pos, ref Vec3 front, ref Vec3 up, ref Mat4 out_mat);
#endregion
#region ID 
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern string EntityNameFromId(ulong entity);

    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern void SetEntityName(ulong entity, string name);
#endregion
#region Transform
    // [MethodImpl(MethodImplOptions.InternalCall)]
    // internal static extern void GetEntityTransform(ulong entity , out Transform transform);

    // [MethodImpl(MethodImplOptions.InternalCall)]
    // internal static extern void SetEntityTransform(ulong entity, ref Transform transform);
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern void GetEntityPosition(ulong entity , out Vec3 position);
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern void SetEntityPosition(ulong entity, ref Vec3 position);
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern void GetEntityScale(ulong entity , out Vec3 scale);
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern void SetEntityScale(ulong entity, ref Vec3 scale);
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern void GetEntityRotation(ulong entity , out Vec3 rotation);

    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern void SetEntityRotation(ulong entity, ref Vec3 rotation);
#endregion
#region Renderable
#endregion
#region TexturedRenderable
#endregion
#region CupeMapRenderable
#endregion
#region RenderableModel
#endregion
#region DirectionLight
#endregion
#region PointLight
#endregion
#region SpotLight
#endregion
#region PhysicsBody
    // [MethodImpl(MethodImplOptions.InternalCall)]
    // internal static extern PhysicsBodyType GetPhysicsBodyType(ulong entity);
    
    // [MethodImpl(MethodImplOptions.InternalCall)]
    // internal static extern void SetPhysicsBodyType(ulong entity, PhysicsBodyType type);
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern void GetPhysicsBodyPosition(ulong entity , out Vec3 position);
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern void SetPhysicsBodyPosition(ulong entity, ref Vec3 position);
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern void GetPhysicsBodyRotation(ulong entity , out Vec3 rotation);
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern void SetPhysicsBodyRotation(ulong entity, ref Vec3 rotation);
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern float GetPhysicsBodyMass(ulong entity);
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern void SetPhysicsBodyMass(ulong entity, float mass);
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern void ApplyForceCenterOfMass(ulong entity , ref Vec3 force);
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern void ApplyForce(ulong entity , ref Vec3 force , ref Vec3 point);
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern void ApplyTorque(ulong entity , ref Vec3 torque);
#endregion
#region Camera
    // [MethodImpl(MethodImplOptions.InternalCall)]
    // internal static extern CameraType GetCameraType(int camera_id);
            
    // [MethodImpl(MethodImplOptions.InternalCall)]
    // internal static extern void SetCameraType(int camera_id, CameraType type);
    
    // camera-scene functions
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern void RecalculateCameraViewProjectionMatrix(int camera_id);
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern void GetCameraView(int camera_id , out Mat4 matrix);
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern void GetCameraProjection(int camera_id , out Mat4 matrix);
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern void GetCameraViewProjection(int camera_id , out Mat4 matrix);
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern void GetCameraPosition(int camera_id , out Vec3 position);
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern void SetCameraPosition(int camera_id, ref Vec3 position);
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern void GetCameraFront(int camera_id , out Vec3 front);
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern void SetCameraFront(int camera_id, ref Vec3 front);
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern void GetCameraUp(int camera_id , out Vec3 up);
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern void SetCameraUp(int camera_id, ref Vec3 up);
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern void GetCameraRight(int camera_id , out Vec3 right);
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern void GetCameraWorldUp(int camera_id , out Vec3 world_up);
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern void SetCameraWorldUp(int camera_id, ref Vec3 world_up);
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern void GetCameraEulerAngles(int camera_id , out Vec3 angles);
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern void SetCameraEulerAngles(int camera_id, ref Vec3 angles);
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern void GetCameraViewportSize(int camera_id , out Vec2 size);
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern void SetCameraViewportSize(int camera_id, ref Vec2 size);
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern void GetCameraClip(int camera_id , out Vec2 clip);
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern void SetCameraClip(int camera_id, ref Vec2 clip);
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern void SetCameraNearClip(int camera_id, float near);
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern void SetCameraFarClip(int camera_id, float far);
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern void GetCameraMousePos(int camera_id , out Vec2 position);
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern void GetCameraLastMousePos(int camera_id , out Vec2 position);
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern void GetCameraDeltaMouse(int camera_id , out Vec2 delta);

    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern float GetCameraSpeed(int camera_id);
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern void SetCameraSpeed(int camera_id, float speed);
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern float GetCameraSensitivity(int camera_id);
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern void SetCameraSensitivity(int camera_id, float sensitivity);
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern float GetCameraFov(int camera_id);
            
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern void SetCameraFov(int camera_id, float fov);
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern float GetCameraZoom(int camera_id);
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern void SetCameraZoom(int camera_id, float zoom);
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern bool GetCameraConstrainPitch(int camera_id);
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern void SetCameraConstrainPitch(int camera_id, bool constrain_pitch);
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern bool IsCameraPrimary(int camera_id);
            
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern void SetCameraPrimary(int camera_id , bool primary);
#endregion
#region Entity 
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern ulong GetEntityParent(ulong entity);

    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern void SetEntityParent(ulong child , ulong parent);
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern void EntityAddComponent(ulong entity, Type type);
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern bool EntityHasComponent(ulong entity, Type type);
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern bool EntityRemoveComponent(ulong entity, Type type);
#endregion
#region Scene
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern ulong CreateEntity(string name);
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern void DestroyEntity(ulong entity);
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern bool IsEntityValid(ulong entity);
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern ulong GetEntityFromName(string name);
#endregion
#region Keyboard
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern int KeyFramesHeld(KeyCode key);
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern bool IsKeyPressed(KeyCode key);
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern bool IsKeyBlocked(KeyCode key);
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern bool IsKeyHeld(KeyCode key);
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern bool IsKeyDown(KeyCode key);
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern bool IsKeyReleased(KeyCode key);
#endregion
#region Mouse
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern void SnapMouseToCenter();
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern void FreeMouse();
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern void HideMouse();

    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern int MouseX();
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern int MouseY();
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern int MouseLastX();
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern int MouseLastY();
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern int MouseDX();
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern int MouseDY();
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern Vec2 MousePos(out Vec2 position);
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern Vec2 MouseLastPos(out Vec2 position);
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern Vec2 MouseDelta(out Vec2 delta);
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern bool MouseInWindow();
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern int ButtonFramesHeld(Button button);
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern bool IsMouseButtonPressed(Button button);
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern bool IsMouseButtonBlocked(Button button);

    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern bool IsMouseButtonHeld(Button button);
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    internal static extern bool IsMouseButtonReleased(Button button);
#endregion
#region Log
    [MethodImpl(MethodImplOptions.InternalCall)]
    public static extern void LogTrace(String line);

    [MethodImpl(MethodImplOptions.InternalCall)]
    public static extern void LogDebug(String line);

    [MethodImpl(MethodImplOptions.InternalCall)]
    public static extern void LogInfo(String line);

    [MethodImpl(MethodImplOptions.InternalCall)]
    public static extern void LogWarn(String line);

    [MethodImpl(MethodImplOptions.InternalCall)]
    public static extern void LogError(String line);

    [MethodImpl(MethodImplOptions.InternalCall)]
    public static extern void LogFatal(String line);
#endregion 
  } 

}
