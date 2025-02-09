using System;

namespace Other {

  public class Camera : Component {
    public Camera() : base(null) {}

    public Camera(OtherObject obj) : base(obj) {
    }
      
    internal static unsafe delegate*<IntPtr , Vec3* , void> GetPosition;
    internal static unsafe delegate*<IntPtr , Vec3* , void> SetPosition;

    internal static unsafe delegate*<IntPtr , Vec3* , void> GetForward;
    ///  TODO: do we need this?
    // internal static unsafe delegate*<IntPtr , Vec3* , void> SetForward;

    internal static unsafe delegate*<IntPtr , Vec3* , void> GetRight;
    /// TODO: do we need this?
    /// internal static unsafe delegate*<IntPtr , Vec3* , void> SetRight;
    
    internal static unsafe delegate*<IntPtr , Vec3* , void> GetUp;
    /// TODO: do we need this?
    /// internal static unsafe delegate*<IntPtr , Vec3* , void> SetUp;

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

    public Vec3 Forward {
      get {
        unsafe {
          Vec3 forward = Vec3.zero;
          GetForward(Object.NativeHandle , &forward);
          return forward;
        }
      }
    }

    public Vec3 Right {
      get {
        unsafe {
          Vec3 right = Vec3.zero;
          GetRight(Object.NativeHandle , &right);
          return right;
        }
      }
    }

    public Vec3 Up {
      get {
        unsafe {
          Vec3 up = Vec3.zero;
          GetUp(Object.NativeHandle , &up);
          return up;
        }
      }
    }

    public void SetTransform(Transform transform) {
      Position = transform.Position;
      /// rotation, etc... 
    }

    // public Vec3 Direction {
    //   get {
    //     GetDirection(Object.ObjectID , out Vec3 direction);
    //     return direction;
    //   }
    //   set => SetDirection(Object.ObjectID , ref value);
    // }

    // public Vec3 Up {
    //   get {
    //     GetUp(Object.ObjectID , out Vec3 up);
    //     return up;
    //   }
    //   set => SetUp(Object.ObjectID , ref value);
    // }

    // public Vec3 Right {
    //   get {
    //     GetRight(Object.ObjectID , out Vec3 right);
    //     return right;
    //   }
    //   set => SetRight(Object.ObjectID , ref value);
    // }

    // public Vec3 WorldUp {
    //   get {
    //     GetWorldUp(Object.ObjectID , out Vec3 worldUp);
    //     return worldUp;
    //   }
    //   set => SetWorldUp(Object.ObjectID , ref value);
    // } 

    // public Vec3 Orientation {
    //   get {
    //     GetOrientation(Object.ObjectID , out Vec3 orientation);
    //     return orientation;
    //   }
    //   set => SetOrientation(Object.ObjectID , ref value);
    // }

    // public float Yaw {
    //   get {
    //     return Orientation.x;
    //   }
    //   set {
    //     Vec3 orientation = Orientation;
    //     orientation.x = value;
    //     Orientation = orientation;
    //   }
    // }

    // public float Pitch {
    //   get {
    //     return Orientation.z;
    //   }
    //   set {
    //     Vec3 orientation = Orientation;
    //     orientation.z = value;
    //     Orientation = orientation;
    //   }
    // }
    // public float Roll {
    //   get {
    //     return Orientation.y;
    //   }
    //   set {
    //     Vec3 orientation = Orientation;
    //     orientation.y = value;
    //     Orientation = orientation;
    //   }
    // }
    // public float Speed {
    //   get {
    //     GetSpeed(Object.ObjectID , out float speed);
    //     return speed;
    //   }
    //   set => SetSpeed(Object.ObjectID , ref value);
    // }
    
    // [MethodImpl(MethodImplOptions.InternalCall)]
    // private static extern void GetDirection(ulong objectID, out Vec3 direction);
    // [MethodImpl(MethodImplOptions.InternalCall)]
    // private static extern void SetDirection(ulong objectID, ref Vec3 direction);

    // [MethodImpl(MethodImplOptions.InternalCall)]
    // private static extern void GetUp(ulong objectID, out Vec3 up);
    // [MethodImpl(MethodImplOptions.InternalCall)]
    // private static extern void SetUp(ulong objectID, ref Vec3 up);

    // [MethodImpl(MethodImplOptions.InternalCall)]
    // private static extern void GetRight(ulong objectID, out Vec3 right);
    // [MethodImpl(MethodImplOptions.InternalCall)]
    // private static extern void SetRight(ulong objectID, ref Vec3 right);

    // [MethodImpl(MethodImplOptions.InternalCall)]
    // private static extern void GetWorldUp(ulong objectID, out Vec3 worldUp);
    // [MethodImpl(MethodImplOptions.InternalCall)]
    // private static extern void SetWorldUp(ulong objectID, ref Vec3 worldUp);

    // [MethodImpl(MethodImplOptions.InternalCall)]
    // private static extern void GetOrientation(ulong objectID, out Vec3 orientation);
    // [MethodImpl(MethodImplOptions.InternalCall)]
    // private static extern void SetOrientation(ulong objectID, ref Vec3 orientation);

    // [MethodImpl(MethodImplOptions.InternalCall)]
    // private static extern void GetSpeed(ulong objectID, out float speed);
    // [MethodImpl(MethodImplOptions.InternalCall)]
    // private static extern void SetSpeed(ulong objectID, ref float speed);
  }

}