using System;
using System.Runtime.InteropServices;

namespace enginey {

  [StructLayout(LayoutKind.Sequential)]
  public struct Vec4 : IEquatable<Vec4> {
    public static Vec4 zero = new Vec4(0.0f, 0.0f , 0.0f , 0.0f);
    public static Vec4 one = new Vec4(1.0f, 1.0f , 1.0f , 1.0f);

    public static Vec4 Infinity = new Vec4(float.PositiveInfinity);

    public float x;
    public float y;
    public float z;
    public float w;

    public Vec4(float scalar) => x = y = z = w = scalar;

    public Vec4(float x, float y , float z , float w) {
      this.x = x;
      this.y = y;
      this.z = z;
      this.w = w;
    }

    public Vec4(Vec4 vec) {
      x = vec.x;
      y = vec.y;
      z = vec.z;
      w = vec.w;
    }

    public Vec4(Vec3 vec) {
      x = vec.x;
      y = vec.y;
      z = vec.z;
      w = 0.0f;
    }

    public Vec4(Vec2 vec) {
      x = vec.x;
      y = vec.y;
      z = 0.0f;
      w = 0.0f;
    }

    public Vec4(Vec3 vec , float w) {
      x = vec.x;
      y = vec.y;
      z = vec.z;
      this.w = w;
    }

    public Vec4(Vec2 vec , float z , float w) {
      x = vec.x;
      y = vec.y;
      this.z = z;
      this.w = w;
    }

    public void Clamp(Vec4 min, Vec4 max) {
      x = Mathf.Clamp(x, min.x, max.x);
      y = Mathf.Clamp(y, min.y, max.y);
      z = Mathf.Clamp(z, min.z, max.z);
      w = Mathf.Clamp(w, min.w, max.w);
    }

    public float Magnitude() => (float)Math.Sqrt(x * x + y * y + z * z + w * w);

    public Vec4 Normalized() {
      float mag = Magnitude();
      if (mag == 0.0f)
        return zero;
      return new Vec4(x / mag, y / mag , z / mag , w / mag);
    }

    public void Normalize() {
      float mag = Magnitude();
      if (mag == 0.0f)
        return;
          
      x /= mag;
      y /= mag;
      z /= mag;
      w /= mag;
    }

    public float Distance(Vec4 other) {
      return (float)Math.Sqrt(Math.Pow(x - other.x, 2) + 
                              Math.Pow(y - other.y, 2) + 
                              Math.Pow(z - other.z, 2) +
                              Math.Pow(w - other.w, 2));
    }

    public static Vec4 Lerp(Vec4 p1 , Vec4 p2 , float t) {
      if (t < 0.0f)
        return p1;
      if (t > 1.0f)
        return p2;
      
      return p1 + ((p2 - p1) * t);
    }

    public static float Dot(Vec4 left, Vec4 right) { return left.x * right.x + left.y * right.y; }

    public static Vec4 operator *(Vec4 left, float scalar) => new Vec4(left.x * scalar, left.y * scalar , left.z * scalar , left.w * scalar );
    public static Vec4 operator *(float scalar, Vec4 right) => new Vec4(scalar * right.x, scalar * right.y , scalar * right.z , scalar * right.w);
    public static Vec4 operator *(Vec4 left, Vec4 right) => new Vec4(left.x * right.x, left.y * right.y , left.z * right.z , left.w * right.w);
    public static Vec4 operator /(Vec4 left, Vec4 right) => new Vec4(left.x / right.x, left.y / right.y , left.z / right.z , left.w / right.w);
    public static Vec4 operator /(Vec4 left, float scalar) => new Vec4(left.x / scalar, left.y / scalar , left.z / scalar , left.w / scalar);
    public static Vec4 operator /(float scalar, Vec4 right) => new Vec4(scalar/ right.x, scalar/ right.y , scalar/ right.z , scalar/ right.w);
    public static Vec4 operator +(Vec4 left, Vec4 right) => new Vec4(left.x + right.x, left.y + right.y , left.z + right.z , left.w + right.w);
    public static Vec4 operator +(Vec4 left, float right) => new Vec4(left.x + right, left.y + right , left.z + right , left.w + right);
    public static Vec4 operator -(Vec4 left, Vec4 right) => new Vec4(left.x - right.x, left.y - right.y , left.z - right.z , left.w - right.w);
    public static Vec4 operator -(Vec4 left, float right) => new Vec4(left.x - right, left.y - right , left.z - right , left.w - right);
    public static Vec4 operator -(Vec4 vector) => new Vec4(-vector.x, -vector.y , -vector.z , -vector.w);
    public override bool Equals(object obj) => obj is Vec4 other && Equals(other);
    public bool Equals(Vec4 right) => x == right.x && y == right.y && z == right.z && w == right.w;

    public override int GetHashCode() => (x , y , z , w).GetHashCode();
    public static bool operator ==(Vec4 left, Vec4 right) => left.Equals(right);
    public static bool operator !=(Vec4 left, Vec4 right) => !(left == right);
    
    public override string ToString() => $"Vec4[{x} , {y} , {z} , {w}]";

  }

}
