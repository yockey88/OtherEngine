using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;
using System.Runtime.Remoting.Activation;

namespace Other {

  [StructLayout(LayoutKind.Sequential)]
  public class Transform : Component {
    public Transform() { }
    public Transform(OtherObject obj) : base(obj) { }

    public Vec3 Scale {
      get {
        NativeGetScale(Object.ObjectID , out Vec3 scale);
        return scale;
      }
      set => NativeSetScale(Object.ObjectID , ref value);
    }

    public Vec3 HalfExtents {
      get {
        return new Vec3(0.5f) * Scale;
      }
    }

    public Vec3 Position {
      get {
        NativeGetPosition(Object.ObjectID , out Vec3 position);
        return position;
      }
      set => NativeSetPosition(Object.ObjectID , ref value);
    }

    public Vec3 Rotation {
      get {
        NativeGetRotation(Object.ObjectID , out Vec3 rotation);
        return rotation;
      }
      set => NativeSetRotation(Object.ObjectID , ref value);
    }

    public Quaternion Quat {
      get {
        NativeGetQuaternion(Object.ObjectID , out Quaternion quaternion);
        return quaternion;
      }
      set => NativeSetQuaternion(Object.ObjectID , ref value);
    }

    public void Rotate(float angle , Vec3 axis) {
      NativeRotateObject(Object.ObjectID , angle , ref axis);
    }

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void NativeGetScale(ulong id , out Vec3 scale);    
    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void NativeGetPosition(ulong id , out Vec3 position);    

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void NativeGetRotation(ulong id , out Vec3 rotation);
    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void NativeSetScale(ulong id , ref Vec3 scale);    
    

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void NativeSetPosition(ulong id , ref Vec3 position);    
    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void NativeSetRotation(ulong id , ref Vec3 rotation);  
    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void NativeRotateObject(ulong id , float angle , ref Vec3 axis);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void NativeGetQuaternion(ulong id , out Quaternion quaternion);
    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void NativeSetQuaternion(ulong id , ref Quaternion quaternion);
  }
    
}