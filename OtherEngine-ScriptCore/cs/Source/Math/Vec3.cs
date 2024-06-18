using System;
using System.Runtime.InteropServices;

namespace Other {

  [StructLayout(LayoutKind.Sequential)]
  public struct Vec3 : IEquatable<Vec3> {
    public static Vec3 zero = new Vec3(0.0f, 0.0f , 0.0f);
    public static Vec3 one = new Vec3(1.0f, 1.0f , 1.0f);
    public static Vec3 up = new Vec3(0.0f, 1.0f , 0.0f);
    public static Vec3 down = new Vec3(0.0f, -1.0f , 0.0f);
    public static Vec3 left = new Vec3(-1.0f, 0.0f , 0.0f);
    public static Vec3 right = new Vec3(1.0f, 0.0f , 0.0f);
    public static Vec3 forward = new Vec3(0.0f, 0.0f , 1.0f);
    public static Vec3 back = new Vec3(0.0f, 0.0f , -1.0f);

    public static Vec3 Infinity = new Vec3(float.PositiveInfinity);

    public float x;
    public float y;
    public float z;

    public Vec3(float scalar) => x = y = z = scalar;

    public Vec3(float x, float y , float z) {
      this.x = x;
      this.y = y;
      this.z = z;
    }

    public Vec3(float x , Vec2 yz) {
      this.x = x;
      y = yz.x;
      z = yz.y;
    }

    public Vec3(Vec2 xy , float z) {
      x = xy.x;
      y = xy.y;
      this.z = z;
    }

    public Vec3(Vec3 vec) {
      x = vec.x;
      y = vec.y;
      z = vec.z;
    }

    public Vec3(Vec4 vec) {
      x = vec.x;
      y = vec.y;
      z = vec.z;
    }

    public void Clamp(Vec3 min, Vec3 max) {
      x = Mathf.Clamp(x, min.x, max.x);
      y = Mathf.Clamp(y, min.y, max.y);
      z = Mathf.Clamp(z, min.z, max.z);
    }

    public float Magnitude() => (float)Math.Sqrt(x * x + y * y + z * z);

    public Vec3 Normalized() {
      float mag = Magnitude();
      if (mag == 0.0f)
        return zero;
      return new Vec3(x / mag , y / mag , z / mag);
    }

    public void Normalize() {
      float mag = Magnitude();
      if (mag == 0.0f)
        return;
      x /= mag;
      y /= mag;
      z /= mag;
    }

    public static Vec3 EulerToDirection(Vec3 rot) {
      float x = Mathf.Cos(rot.y) * Mathf.Cos(rot.x);
      float y = Mathf.Sin(rot.y) * Mathf.Cos(rot.x);
      float z = Mathf.Sin(rot.z);

      return new Vec3(x, y, z);
    }

    public float Distance(Vec3 other) {
      return (float)Math.Sqrt(Math.Pow(x - other.x, 2) + 
                              Math.Pow(y - other.y, 2) + 
                              Math.Pow(z - other.z, 2));
    }

    public static Vec3 Lerp(Vec3 p1 , Vec3 p2 , float t) {
      if (t < 0.0f)
        return p1;
      if (t > 1.0f)
        return p2;
      
      return p1 + ((p2 - p1) * t);
    }

    public const float kEpsilon = 1e-15f;

    public float SqrtMag { get { return x * x + y * y + z * z; } }

    public static float Dot(Vec3 lhs , Vec3 rhs) { return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z; }

    public static float Angle(Vec3 from , Vec3 to) {
      float denominator = (float)Math.Sqrt(from.SqrtMag * to.SqrtMag);
      if (denominator < kEpsilon)
        return 0.0f;
          
      float dot = Dot(from.Normalized(), to.Normalized());
      return (float)Math.Acos(Mathf.Clamp(dot / denominator , -1.0f , 1.0f)) * Mathf.rad2deg;
    }

    public static Vec3 operator *(Vec3 left, float scalar) => new Vec3(left.x * scalar, left.y * scalar , left.z * scalar);
    public static Vec3 operator *(float scalar, Vec3 right) => new Vec3(scalar * right.x, scalar * right.y , scalar * right.z);
    public static Vec3 operator *(Vec3 left, Vec3 right) => new Vec3(left.x * right.x, left.y * right.y , left.z * right.z);
    public static Vec3 operator /(Vec3 left, Vec3 right) => new Vec3(left.x / right.x, left.y / right.y , left.z / right.z);
    public static Vec3 operator /(Vec3 left, float scalar) => new Vec3(left.x / scalar, left.y / scalar , left.z / scalar);
    public static Vec3 operator /(float scalar, Vec3 right) => new Vec3(scalar/ right.x, scalar/ right.y , scalar/ right.z);
    public static Vec3 operator +(Vec3 left, Vec3 right) => new Vec3(left.x + right.x, left.y + right.y , left.z + right.z);
    public static Vec3 operator +(Vec3 left, float right) => new Vec3(left.x + right, left.y + right , left.z + right);
    public static Vec3 operator -(Vec3 left, Vec3 right) => new Vec3(left.x - right.x, left.y - right.y , left.z - right.z);
    public static Vec3 operator -(Vec3 left, float right) => new Vec3(left.x - right, left.y - right , left.z - right);
    public static Vec3 operator -(Vec3 vector) => new Vec3(-vector.x, -vector.y , -vector.z);

    public static Vec3 operator*(Mat4 lhs, Vec3 rhs) {
      return new Vec3(lhs.a00 * rhs.x + lhs.a01 * rhs.y + lhs.a02 * rhs.z + lhs.a03,
                      lhs.a10 * rhs.x + lhs.a11 * rhs.y + lhs.a12 * rhs.z + lhs.a13,
                      lhs.a20 * rhs.x + lhs.a21 * rhs.y + lhs.a22 * rhs.z + lhs.a23);
    }

    public static Vec3 Cross(Vec3 lhs , Vec3 rhs) {
      return new Vec3(lhs.y * rhs.z - lhs.z * rhs.y,
                      lhs.z * rhs.x - lhs.x * rhs.z,
                      lhs.x * rhs.y - lhs.y * rhs.x);
    }
    public override bool Equals(object obj) => obj is Vec3 other && Equals(other);
    public bool Equals(Vec3 right) => x == right.x && y == right.y && z == right.z;
    public override int GetHashCode() => (x , y , z).GetHashCode();
    public static bool operator ==(Vec3 left, Vec3 right) => left.Equals(right);
    public static bool operator !=(Vec3 left, Vec3 right) => !(left == right);
    public static Vec3 Cos(Vec3 vec) => new Vec3(Mathf.Cos(vec.x) , Mathf.Cos(vec.y) , Mathf.Cos(vec.z));
    public static Vec3 Sin(Vec3 vec) => new Vec3(Mathf.Sin(vec.x) , Mathf.Sin(vec.y) , Mathf.Sin(vec.z));

    public static Vec3 ClampMagnitude(Vec3 vec , float max) {
      float mag = vec.Magnitude();
      if (mag > max)
        return vec.Normalized() * max;
      return vec;
    }
    	
    public override string ToString() => $"Vec3[{x} , {y} , {z}]";

    public Vec2 xy {
      get => new Vec2(x , y);
      set {
        x = value.x;
        y = value.y;
      }
    }

    public Vec2 xz {
      get => new Vec2(x , z);
      set {
        x = value.x;
        z = value.y;
      }
    }

    public Vec2 yz {
      get => new Vec2(y , z);
      set {
        y = value.x;
        z = value.y;
      }
    }
  }

}
