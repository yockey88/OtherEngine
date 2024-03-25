using System;
using System.Runtime.InteropServices;

namespace enginey {
    
  [StructLayout(LayoutKind.Explicit)]
  public struct Mat4 {
    [FieldOffset(0)]  public float a00;
    [FieldOffset(4)]  public float a01;
    [FieldOffset(8)]  public float a02;
    [FieldOffset(12)] public float a03;
    [FieldOffset(16)] public float a10;
    [FieldOffset(20)] public float a11;
    [FieldOffset(24)] public float a12;
    [FieldOffset(28)] public float a13;
    [FieldOffset(32)] public float a20;
    [FieldOffset(36)] public float a21;
    [FieldOffset(40)] public float a22;
    [FieldOffset(44)] public float a23;
    [FieldOffset(48)] public float a30;
    [FieldOffset(52)] public float a31;
    [FieldOffset(56)] public float a32;
    [FieldOffset(60)] public float a33;
    
    // initialize to diagonal matrix
    public Mat4(float value) {
      a00 = value; a01 = 0f;    a02 = 0f;    a03 = 0f;
      a10 = 0f;    a11 = value; a12 = 0f;    a13 = 0f;
      a20 = 0f;    a21 = 0f;    a22 = value; a23 = 0f;
      a30 = 0f;    a31 = 0f;    a32 = 0f;    a33 = value;
    }
    
    public Mat4 Translate(Vec3 position) {
      return new Mat4(1f) {
        a03 = position.x ,
        a13 = position.y ,
        a23 = position.z
      };
    }

    public Mat4 Scale(Vec3 size) {
      return new Mat4(1f) {
        a00 = size.x ,
        a11 = size.y ,
        a22 = size.z
      };
    }

    public Mat4 Scale(float scalar) {
      return new Mat4(1f) {
        a00 = scalar ,
        a11 = scalar ,
        a22 = scalar 
      };
    }
    
    public Vec3 translation {
      get => new Vec3(a03 , a13 , a23);
      set { a03 = value.x; a13 = value.y; a23 = value.z; }
    }

    public Vec3 scale {
      get {
        var sx = new Vec3(a00, a10, a20);
        var sy = new Vec3(a01, a11, a21);
        var sz = new Vec3(a02, a12, a22);
        return new Vec3(sx.Magnitude() , sy.Magnitude(), sz.Magnitude());
      }
      set { a00 = value.x; a11 = value.y; a22 = value.z; }
    }

    public Vec3 rotation {
      get {
        if (Mathf.Abs(a00 - 1f) < float.Epsilon || 
            Mathf.Abs(a00 + 1f) < float.Epsilon) {
          return new Vec3(
            Mathf.Atan2(a02 , a23) ,
            0f ,
            0f
          );
        } else {
          return new Vec3(
            Mathf.Atan2(-a12 , a00) ,
            Mathf.Asin(a10) ,
            Mathf.Atan2(-a12 , a11)
          );
        }
      }
    }

    public Mat4 LookAt(Vec3 pos, Vec3 front, Vec3 up) {
      Mat4 result = new Mat4();
      Engine.Mat4LookAt(ref pos, ref front, ref up, ref result);
      return result;
    }
  }
    
}
