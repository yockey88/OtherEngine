using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;

namespace Other {

  public class Scene {

    private static Dictionary<ulong , OtherObject> objects = new Dictionary<ulong , OtherObject>();
    
    /// native getters    
    [MethodImpl(MethodImplOptions.InternalCall)]
    public static extern string NativeGetName(ulong id);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void NativeGetScale(ulong id , out Vec3 scale);    

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void NativeGetPosition(ulong id , out Vec3 position);    

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void NativeGetRotation(ulong id , out Vec3 rotation);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern ulong NativeGetObjectByName(string name);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern bool NativeHasObjectById(ulong id);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern bool NativeHasObjectByName(string name);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern ulong NativeGetParent(ulong id);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern ulong NativeGetNumChildren(ulong id);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern ulong[] NativeGetChildren(ulong id);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern UInt32 NativeGetPhysicsBodyType(ulong id);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern float NativeGetMass(ulong id);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern float NativeGetLinearDrag(ulong id);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern float NativeGetAngularDrag(ulong id);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern float NativeGetGravityScale(ulong id);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern bool NativeGetFixedRotation(ulong id);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern bool NativeGetBullet(ulong id);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void NativeGet2DColliderOffset(ulong id , out Vec2 offset);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void NativeGet2DColliderSize(ulong id , out Vec2 size);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern float NativeGet2DColliderDensity(ulong id);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern float NativeGet2DColliderFriction(ulong id);

    // [MethodImpl(MethodImplOptions.InternalCall)]
    // private static extern void NativeGetMeshHandle(ulong id , out UUID handle);
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern UInt64 NativeGetStaticMeshHandle(ulong id);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void NativeGetStaticMeshMaterial(ulong id , out Material material);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void NativeAddComponent(ulong id , Type type);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern bool NativeHasComponent(ulong id , Type type);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void NativeRemoveComponent(ulong id , Type type);

    /// C# getters
    static public string GetName(ulong id) {
      return NativeGetName(id);
    }

    static public void GetPosition(ulong id , out Vec3 position) {
      NativeGetPosition(id , out position);
    }

    static public void GetScale(ulong id , out Vec3 scale) {
      NativeGetScale(id , out scale);
    }

    static public void GetRotation(ulong id , out Vec3 rotation) {
      NativeGetRotation(id , out rotation);
    }

    static public OtherObject GetObject(ulong id) {
      if (!HasObject(id)) {
        return null;
      }

      if (objects.ContainsKey(id)) {
        return objects[id];
      } else {
        OtherObject obj = new OtherObject();
        obj.ObjectID = id;
        objects.Add(id , obj);
        return obj;
      }
    }

    static public OtherObject GetObject(string name) {
      if (!HasObject(name)) {
        return null;
      }

      ulong id = NativeGetObjectByName(name);
      if (objects.ContainsKey(id)) {
        return objects[id];
      } else {
        OtherObject obj = new OtherObject();
        obj.ObjectID = id;
        objects.Add(id , obj);
        return obj;
      }
    }

    static public bool HasObject(ulong id) {
      return NativeHasObjectById(id);
    }

    static public bool HasObject(string name) {
      return NativeHasObjectByName(name);
    }

    static public OtherObject GetParent(ulong id) {
      ulong pid = NativeGetParent(id);
      if (objects.ContainsKey(pid)) {
        return objects[pid];
      } else if (pid == 0) {
        return null;
      }

      OtherObject obj = new OtherObject();
      obj.ObjectID = pid;
      objects.Add(pid , obj);
      return null;
    }

    static public ulong GetNumChildren(ulong id) {
      return NativeGetNumChildren(id);
    }

    static public List<OtherBehavior> GetChildren(ulong id) {
      ulong[] ids = NativeGetChildren(id);

      List<OtherBehavior> children = new List<OtherBehavior>();

      foreach (ulong child_id in ids) {
        if (objects.ContainsKey(child_id)) {
          children.Add(objects[child_id]);
        } else {
          OtherObject obj = new OtherObject{ ObjectID = child_id };
          objects.Add(child_id , obj);
          children.Add(obj);
        }
      }

      return children;
    }

    static public PhysicsBodyType GetPhysicsBodyType(ulong id) {
      return (PhysicsBodyType)NativeGetPhysicsBodyType(id);
    }

    static public float GetMass(ulong id) {
      return NativeGetMass(id);
    }

    static public float GetLinearDrag(ulong id) {
      return NativeGetLinearDrag(id);
    }

    static public float GetAngularDrag(ulong id) {
      return NativeGetAngularDrag(id);
    }

    static public float GetGravityScale(ulong id) {
      return NativeGetGravityScale(id);
    }

    static public bool GetFixedRotation(ulong id) {
      return NativeGetFixedRotation(id);
    }

    static public bool GetBullet(ulong id) {
      return NativeGetBullet(id);
    }

    static public void GetColliderOffset(ulong id , out Vec2 offset) {
      NativeGet2DColliderOffset(id , out offset);
    }

    static public void GetColliderSize(ulong id , out Vec2 size) {
      NativeGet2DColliderSize(id , out size);
    }

    static public float GetDensity(ulong id) {
      return NativeGet2DColliderDensity(id);
    }

    static public float GetFriction(ulong id) {
      return NativeGet2DColliderFriction(id);
    }

    // static public void GetMeshHandle(ulong id , out UUID handle) {
    //   NativeGetMeshHandle(id , out handle);
    // }

    static public UUID GetStaticMeshHandle(ulong id) {
      return NativeGetStaticMeshHandle(id);
    }

    static public void GetStaticMeshMaterial(ulong id , out Material material) {
      NativeGetStaticMeshMaterial(id , out material);
    }

    static public void AddComponent(ulong id , Type type) {
      NativeAddComponent(id , type);
    }

    static public bool HasComponent(ulong id , Type type) {
      return NativeHasComponent(id , type);
    }

    static public void RemoveComponent(ulong id , Type type) {
      NativeRemoveComponent(id , type);
    }

    /// native setters

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void NativeSetScale(ulong id , ref Vec3 scale);    
    

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void NativeSetPosition(ulong id , ref Vec3 position);    

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void NativeSetRotation(ulong id , ref Vec3 rotation);  

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void NativeRotateObject(ulong id , float angle , ref Vec3 axis);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void NativeSetPhysicsBodyType(ulong id , UInt32 type); 

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void NativeSetMass(ulong id , float mass);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void NativeSetLinearDrag(ulong id , float drag);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void NativeSetAngularDrag(ulong id , float drag);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void NativeSetGravityScale(ulong id , float scale);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void NativeSetFixedRotation(ulong id , bool fixed_rotation);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void NativeSetBullet(ulong id , bool bullet);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void NativeSet2DColliderOffset(ulong id , ref Vec2 offset);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void NativeSet2DColliderSize(ulong id , ref Vec2 size);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void NativeSet2DColliderDensity(ulong id , float density);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void NativeSet2DColliderFriction(ulong id , float friction);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void NativeSetStaticMeshHandle(ulong id , UInt64 handle);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void NativeSetStaticMeshMaterial(ulong id , ref Material material);

    // [MethodImpl(MethodImplOptions.InternalCall)]
    // private static extern void NativeSetParent(ulong id , ref Vec3 scale);

    /// C# setters 

    static public void SetPosition(ulong id , ref Vec3 position) {
      NativeSetPosition(id , ref position);
    }

    static public void SetScale(ulong id , ref Vec3 scale) {
      NativeSetScale(id , ref scale);
    }

    static public void SetRotation(ulong id , ref Vec3 rotation) {
      NativeSetRotation(id , ref rotation);
    }

    static public void RotateObject(ulong id , float angle , ref Vec3 axis) {
      NativeRotateObject(id , angle , ref axis);
    }

    static public void SetPhysicsBodyType(ulong id , PhysicsBodyType type) {
      NativeSetPhysicsBodyType(id , (UInt32)type);
    }

    static public void SetMass(ulong id , float mass) {
      NativeSetMass(id , mass);
    }

    static public void SetLinearDrag(ulong id , float drag) {
      NativeSetLinearDrag(id , drag);
    }

    static public void SetAngularDrag(ulong id , float drag) {
      NativeSetAngularDrag(id , drag);
    }

    static public void SetGravityScale(ulong id , float scale) {
      NativeSetGravityScale(id , scale);
    }

    static public void SetFixedRotation(ulong id , bool fixed_rotation) {
      NativeSetFixedRotation(id , fixed_rotation);
    }

    static public void SetBullet(ulong id , bool bullet) {
      NativeSetBullet(id , bullet);
    }

    static public void SetColliderOffset(ulong id , ref Vec2 offset) {
      NativeSet2DColliderOffset(id , ref offset);
    }

    static public void SetColliderSize(ulong id , ref Vec2 size) {
      NativeSet2DColliderSize(id , ref size);
    }

    static public void SetDensity(ulong id , float density) {
      NativeSet2DColliderDensity(id , density);
    }

    static public void SetFriction(ulong id , float friction) {
      NativeSet2DColliderFriction(id , friction);
    }

    static public void SetStaticMeshHandle(ulong id , ulong handle) {
      NativeSetStaticMeshHandle(id , handle);
    }

    static public void SetStaticMeshMaterial(ulong id , ref Material material) {
      NativeSetStaticMeshMaterial(id , ref material);
    }

    static public void InitializeScene() {
      try {
        objects.Clear();
      } catch (Exception e) {
        Logger.WriteError($"Failed to initialize scene: {e.Message}");
      }
    }

    /// other
    static public void AddObject(OtherObject obj) {
      try {
        if (objects.ContainsKey(obj.ObjectID)) {
          return;
        }
        objects.Add(obj.ObjectID , obj);
      } catch (Exception e) {
        Logger.WriteError($"Failed to add object to scene: {e.Message}");
      }
    }

    static public void ClearObjects() {
      try {
        objects.Clear();
      } catch (Exception e) {
        Logger.WriteError($"Failed to clear objects: {e.Message}");
      }
    }

  }

}