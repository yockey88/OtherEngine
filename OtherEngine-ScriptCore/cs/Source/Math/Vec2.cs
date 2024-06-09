using System;
using System.Runtime.InteropServices;

namespace Other {

  [StructLayout(LayoutKind.Sequential)]
  public struct Vec2 : IEquatable<Vec2> {
    public static Vec2 zero = new Vec2(0.0f, 0.0f);
    public static Vec2 one = new Vec2(1.0f, 1.0f);
    public static Vec2 up = new Vec2(0.0f, 1.0f);
    public static Vec2 down = new Vec2(0.0f, -1.0f);
    public static Vec2 left = new Vec2(-1.0f, 0.0f);
    public static Vec2 right = new Vec2(1.0f, 0.0f);

    public float x;
    public float y;

    public Vec2(float scalar) => x = y = scalar;

    public Vec2(float x, float y) {
      this.x = x;
      this.y = y;
    }

    public Vec2(Vec2 vec) {
      x = vec.x;
      y = vec.y;
    }

    public Vec2(Vec3 vec) {
      x = vec.x;
      y = vec.y;
    }

    public void Clamp(Vec2 min, Vec2 max) {
      x = Mathf.Clamp(x, min.x, max.x);
      y = Mathf.Clamp(y, min.y, max.y);
    }

    public float  Magnitude() => (float)Math.Sqrt(x * x + y * y);

    public Vec2 Normalized() {
      float mag = Magnitude();
      if (mag == 0.0f)
        return zero;
      return new Vec2(x / mag, y / mag);
    }

    public void Normalize() {
      float mag = Magnitude();
      if (mag == 0.0f)
        return;
          
      x /= mag;
      y /= mag;
    }

    public float Distance(Vec2 other) {
      return (float)Math.Sqrt(Math.Pow(x - other.x, 2) + 
                                Math.Pow(y - other.y, 2));
    }

    public static Vec2 Lerp(Vec2 p1 , Vec2 p2 , float t) {
      if (t < 0.0f)
        return p1;
      if (t > 1.0f)
        return p2;
      
      return p1 + ((p2 - p1) * t);
    }

    public static float Dot(Vec2 left, Vec2 right) { return left.x * right.x + left.y * right.y; }
    public static Vec2 operator *(Vec2 left, float scalar) => new Vec2(left.x * scalar, left.y * scalar);
    public static Vec2 operator *(float scalar, Vec2 right) => new Vec2(scalar * right.x, scalar * right.y);
    public static Vec2 operator *(Vec2 left, Vec2 right) => new Vec2(left.x * right.x, left.y * right.y);
    public static Vec2 operator /(Vec2 left, Vec2 right) => new Vec2(left.x / right.x, left.y / right.y);
    public static Vec2 operator /(Vec2 left, float scalar) => new Vec2(left.x / scalar, left.y / scalar);
    public static Vec2 operator /(float scalar, Vec2 right) => new Vec2(scalar/ right.x, scalar/ right.y);
    public static Vec2 operator +(Vec2 left, Vec2 right) => new Vec2(left.x + right.x, left.y + right.y);
    public static Vec2 operator +(Vec2 left, float right) => new Vec2(left.x + right, left.y + right);
    public static Vec2 operator -(Vec2 left, Vec2 right) => new Vec2(left.x - right.x, left.y - right.y);
    public static Vec2 operator -(Vec2 left, float right) => new Vec2(left.x - right, left.y - right);
    public static Vec2 operator -(Vec2 vector) => new Vec2(-vector.x, -vector.y);
    public override bool Equals(object obj) => obj is Vec2 other && Equals(other);
    public bool Equals(Vec2 right) => x == right.x && y == right.y;
    public override int GetHashCode() => (x , y).GetHashCode();
    public static bool operator ==(Vec2 left, Vec2 right) => left.Equals(right);
    public static bool operator !=(Vec2 left, Vec2 right) => !(left == right);
    public override string ToString() => $"Vec2[{x} , {y}]";
  }

}
