using System;
using System.Runtime.InteropServices;

namespace Other {
    
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

    public static Mat4 Identity => new Mat4(1f);
    
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

    // public Mat4 Scale(Vec3 size) {
    //   return new Mat4(1f) {
    //     a00 = size.x ,
    //     a11 = size.y ,
    //     a22 = size.z
    //   };
    // }

    // public Mat4 Scale(float scalar) {
    //   return new Mat4(1f) {
    //     a00 = scalar ,
    //     a11 = scalar ,
    //     a22 = scalar 
    //   };
    // }
    
    public Vec3 Translation {
      get => new Vec3(a03 , a13 , a23);
      set { a03 = value.x; a13 = value.y; a23 = value.z; }
    }

    public Vec3 Scale {
      get {
        var sx = new Vec3(a00, a10, a20);
        var sy = new Vec3(a01, a11, a21);
        var sz = new Vec3(a02, a12, a22);
        return new Vec3(sx.Magnitude() , sy.Magnitude(), sz.Magnitude());
      }
      set { a00 = value.x; a11 = value.y; a22 = value.z; }
    }

    public Vec3 Rotation {
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

    public Vec4 Column(int index) {
      switch (index) {
        case 0: return new Vec4(a00 , a10 , a20, a30);
        case 1: return new Vec4(a01 , a11 , a21, a31);
        case 2: return new Vec4(a02 , a12 , a22, a32);
        case 3: return new Vec4(a03 , a13 , a23, a33);
        default: throw new IndexOutOfRangeException();
      }
    }

    public Vec4 Row(int index) {
      switch (index) {
        case 0: return new Vec4(a00 , a01 , a02, a03);
        case 1: return new Vec4(a10 , a11 , a12, a13);
        case 2: return new Vec4(a20 , a21 , a22, a23);
        case 3: return new Vec4(a30 , a31 , a32, a33);
        default: throw new IndexOutOfRangeException();
      }
    }

    // public Mat4 LookAt(Vec3 pos, Vec3 front, Vec3 up) {
      // Mat4 result = new Mat4();
      // Engine.Mat4LookAt(ref pos, ref front, ref up, ref result);
    //   return result;
    // }

    public static Mat4 operator *(Mat4 left, Mat4 right) {
      return new Mat4() {
        a00 = left.a00 * right.a00 + left.a01 * right.a10 + left.a02 * right.a20 + left.a03 * right.a30,
        a01 = left.a00 * right.a01 + left.a01 * right.a11 + left.a02 * right.a21 + left.a03 * right.a31,
        a02 = left.a00 * right.a02 + left.a01 * right.a12 + left.a02 * right.a22 + left.a03 * right.a32,
        a03 = left.a00 * right.a03 + left.a01 * right.a13 + left.a02 * right.a23 + left.a03 * right.a33,

        a10 = left.a10 * right.a00 + left.a11 * right.a10 + left.a12 * right.a20 + left.a13 * right.a30,
        a11 = left.a10 * right.a01 + left.a11 * right.a11 + left.a12 * right.a21 + left.a13 * right.a31,
        a12 = left.a10 * right.a02 + left.a11 * right.a12 + left.a12 * right.a22 + left.a13 * right.a32,
        a13 = left.a10 * right.a03 + left.a11 * right.a13 + left.a12 * right.a23 + left.a13 * right.a33,

        a20 = left.a20 * right.a00 + left.a21 * right.a10 + left.a22 * right.a20 + left.a23 * right.a30,
        a21 = left.a20 * right.a01 + left.a21 * right.a11 + left.a22 * right.a21 + left.a23 * right.a31,
        a22 = left.a20 * right.a02 + left.a21 * right.a12 + left.a22 * right.a22 + left.a23 * right.a32,
        a23 = left.a20 * right.a03 + left.a21 * right.a13 + left.a22 * right.a23 + left.a23 * right.a33,

        a30 = left.a30 * right.a00 + left.a31 * right.a10 + left.a32 * right.a20 + left.a33 * right.a30,
        a31 = left.a30 * right.a01 + left.a31 * right.a11 + left.a32 * right.a21 + left.a33 * right.a31,
        a32 = left.a30 * right.a02 + left.a31 * right.a12 + left.a32 * right.a22 + left.a33 * right.a32,
        a33 = left.a30 * right.a03 + left.a31 * right.a13 + left.a32 * right.a23 + left.a33 * right.a33
      };
    }

    public override string ToString() => $"[{a00} , {a01} , {a02} , {a03}]\n" +
                                         $"[{a10} , {a11} , {a12} , {a13}]\n" +
                                         $"[{a20} , {a21} , {a22} , {a23}]\n" +
                                         $"[{a30} , {a31} , {a32} , {a33}]\n";
  }
    
}
