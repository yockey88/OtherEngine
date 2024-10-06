using System;
using System.Runtime.CompilerServices;

using DotOther.Managed.Interop;

namespace Other {
  
  public class Transform : Component {
    public Transform() : base(null) { }
    public Transform(OtherObject obj) : base(obj) { }

    internal static unsafe delegate*<IntPtr , Vec3* , void> GetPosition;
    internal static unsafe delegate*<IntPtr , Vec3* , void> SetPosition;

    internal static unsafe delegate*<IntPtr , Vec3*, void> GetRotation;
    internal static unsafe delegate*<IntPtr , Vec3*, void> SetRotation;
    
    internal static unsafe delegate*<IntPtr , Vec3* , void> GetScale;
    internal static unsafe delegate*<IntPtr , Vec3* , void> SetScale;

    internal static unsafe delegate*<IntPtr , float , Vec3* , void> NativeRotate;

    public Vec3 Position {
      get {
        unsafe {
          Vec3 position = Vec3.zero;
          GetPosition(Object.NativeHandle , &position);
          return position;
        }
      }
      set {
        unsafe { 
          SetPosition(Object.NativeHandle , &value); 
        } 
      }
    }

    public Vec3 Rotation {
      get {
        unsafe {
          Vec3 rotation = Vec3.zero;
          GetRotation(Object.NativeHandle , &rotation);
          return rotation;
        }
      }
      set {
        unsafe { 
          SetRotation(Object.NativeHandle , &value); 
        } 
      }
    }

    public Vec3 Scale {
      get {
        unsafe {
          Vec3 scale = Vec3.zero;
          GetScale(Object.NativeHandle , &scale);
          return scale;
        }
      }
      set {
        unsafe { 
          SetScale(Object.NativeHandle , &value); 
        } 
      }
    }

    public void Rotate(float radians , Vec3 axis) {
      unsafe {
        NativeRotate(Object.NativeHandle , radians , &axis);
      }
    }

    // public Quaternion Quat {
    //   get {
    //     NativeGetQuaternion(Object.ObjectID , out Quaternion quaternion);
    //     return quaternion;
    //   }
    //   set => NativeSetQuaternion(Object.ObjectID , ref value);
    // }
  }
    
}