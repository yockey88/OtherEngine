using System;
using System.Runtime.InteropServices;

namespace Other {

  [StructLayout(LayoutKind.Sequential)]
  public struct CollisionPointData {
    private UInt64 other_entity;
    private Vec3 point;

    public UInt64 Object {
      get { return other_entity; }
    }

    public Vec3 Point {
      get { return point; }
    }
  };

  public struct PhysicsRaycastHit {
    public RigidBody body;
    public Collider collider;

    public float distance;
    public Vec3 normal;
    public Vec3 point;

    public PhysicsRaycastHit() {
      body = null;
      collider = null;
      distance = 0.0f;
      normal = Vec3.zero;
      point = Vec3.zero;
    }
  }

  public class Physics {
    internal static unsafe delegate*<Ray*, float*, UInt64*, Vec3*, Vec3*, float*, bool> NativeRaycast;

    public static bool Raycast(Vec3 origin, Vec3 direction, float distance, out PhysicsRaycastHit hit) {
      unsafe {
        PhysicsRaycastHit temp_hit = new PhysicsRaycastHit();
        Ray ray = new Ray();
        ray.origin = origin;
        ray.direction = direction;
        
        UInt64 hit_entity = 0;
        bool did_hit = NativeRaycast(&ray, &distance, &hit_entity, &temp_hit.normal, &temp_hit.point, &temp_hit.distance);
        if (!did_hit) {
          /// make sure everything is safely defaulted after coming back from the native call if we didn't hit anything
          hit = new PhysicsRaycastHit();
        } else {
          hit = temp_hit;
        }

        Logger.WriteDebug($"Physics.Raycast: did_hit: {did_hit}, hit_entity: {hit_entity}, hit.distance: {hit.distance}, hit.normal: {hit.normal}, hit.point: {hit.point}");
        return did_hit;
      }
    }
  }
  
}