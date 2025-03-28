using System;

namespace Other {

  public class RigidBody : Component {
    public RigidBody() : base(null) {}

    public RigidBody(OtherObject obj) : base(obj) {}

    internal static unsafe delegate*<IntPtr, Vec3*, Vec3*, float, void> NativeApplyForce;

    internal static unsafe delegate*<IntPtr, Vec3*, void> GetPosition;
    internal static unsafe delegate*<IntPtr, Vec3*, void> SetPosition;

    public Vec3 Position {
      get {
        unsafe {
          Vec3 pos = Vec3.zero;
          GetPosition(Object.NativeHandle , &pos);
          return pos;
        }
      }
      set {
        unsafe {
          SetPosition(Object.NativeHandle , &value);
        }
      }
    }

    // void ApplyForce(Vec3 position, Vec3 direction, float force_newtons) {
    //   unsafe {
    //     Vec3* pos = &position;
    //     Vec3* dir = &direction;
    //     NativeApplyForce(Object.NativeHandle , pos , dir , force_newtons);
    //   }
    // }
  }

}