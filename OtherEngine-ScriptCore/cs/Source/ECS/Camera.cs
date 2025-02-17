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
    
    internal static unsafe delegate*<IntPtr , Vec3* , void> GetWorldUp;

    internal static unsafe delegate*<IntPtr, float*, void> GetYaw;
    internal static unsafe delegate*<IntPtr, float*, void> SetYaw;

    internal static unsafe delegate*<IntPtr, float*, void> GetPitch;
    internal static unsafe delegate*<IntPtr, float*, void> SetPitch;
    internal static unsafe delegate*<IntPtr, bool*, void> GetIsPitchConstrained;
    internal static unsafe delegate*<IntPtr, bool*, void> SetIsPitchConstrained;
    
    internal static unsafe delegate*<IntPtr, float*, void> GetRoll;
    internal static unsafe delegate*<IntPtr, float*, void> SetRoll;

    internal static unsafe delegate*<IntPtr, float*, void> GetSensitivity;
    internal static unsafe delegate*<IntPtr, float*, void> SetSensitivity;

    internal static unsafe delegate*<IntPtr, void> InternalCalculateMatrix;

    internal static unsafe delegate*<IntPtr, Vec2*, void> SetClipPlanes;
    internal static unsafe delegate*<IntPtr, Vec2*, void> GetClipPlanes;

    internal static unsafe delegate*<IntPtr, float*, void> SetFarClipPlane;
    internal static unsafe delegate*<IntPtr, float*, void> GetFarClipPlane;

    internal static unsafe delegate*<IntPtr, float*, void> SetNearClipPlane;
    internal static unsafe delegate*<IntPtr, float*, void> GetNearClipPlane;

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

    public Vec3 WorldUp {
      get {
        unsafe {
          Vec3 worldUp = Vec3.zero;
          GetWorldUp(Object.NativeHandle , &worldUp);
          return worldUp;
        }
      }
    }

    public float Yaw {
      get {
        unsafe {
          float yaw = 0.0f;
          GetYaw(Object.NativeHandle , &yaw);
          return yaw;
        }
      }
      set {
        unsafe {
          SetYaw(Object.NativeHandle , &value);
        }
      }
    }

    public float Pitch {
      get {
        unsafe {
          float pitch = 0.0f;
          GetPitch(Object.NativeHandle , &pitch);
          return pitch;
        }
      }
      set {
        unsafe {
          SetPitch(Object.NativeHandle , &value);
        }
      }
    }

    public float Roll {
      get {
        unsafe {
          float roll = 0.0f;
          GetRoll(Object.NativeHandle , &roll);
          return roll;
        }
      }
      set {
        unsafe {
          SetRoll(Object.NativeHandle , &value);
        }
      }
    }

    public float Sensitivity {
      get {
        unsafe {
          float sensitivity = 0.0f;
          GetSensitivity(Object.NativeHandle , &sensitivity);
          return sensitivity;
        }
      }
      set {
        unsafe {
          SetSensitivity(Object.NativeHandle , &value);
        }
      }
    }

    public bool PitchConstrained {
      get {
        unsafe {
          bool isPitchConstrained = false;
          GetIsPitchConstrained(Object.NativeHandle , &isPitchConstrained);
          return isPitchConstrained;
        }
      }
      set {
        unsafe {
          SetIsPitchConstrained(Object.NativeHandle , &value);
        }
      }
    }

    public Vec2 ClipPlanes {
      get {
        unsafe {
          Vec2 clipPlanes = Vec2.zero;
          GetClipPlanes(Object.NativeHandle , &clipPlanes);
          return clipPlanes;
        }
      }
      set {
        unsafe {
          SetClipPlanes(Object.NativeHandle , &value);
        }
      }
    }

    public float NearClipPlane {
      get {
        unsafe {
          float nearClipPlane = 0.0f;
          GetNearClipPlane(Object.NativeHandle , &nearClipPlane);
          return nearClipPlane;
        }
      }
      set {
        unsafe {
          SetNearClipPlane(Object.NativeHandle , &value);
        }
      }
    }

    public float FarClipPlane {
      get {
        unsafe {
          float farClipPlane = 0.0f;
          GetFarClipPlane(Object.NativeHandle , &farClipPlane);
          return farClipPlane;
        }
      }
      set {
        unsafe {
          SetFarClipPlane(Object.NativeHandle , &value);
        }
      }
    }

    public void SetTransform(Transform transform) {
      Position = transform.Position;
      /// rotation, etc... 
    }

    public void CalculateMatrix() {
      unsafe {
        InternalCalculateMatrix(Object.NativeHandle);
      }
    }

    public void RotateHorizontal(float angle) {
      Yaw += angle;
    }

    public void RotateVertical(float angle) {
      Pitch += angle;
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