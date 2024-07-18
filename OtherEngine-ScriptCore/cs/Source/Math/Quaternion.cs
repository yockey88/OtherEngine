using System;
using System.Runtime.InteropServices;

namespace Other {

  [StructLayout(LayoutKind.Sequential)]
  public class Quaternion : IEquatable<Quaternion> {
    public float x;
    public float y;
    public float z;
    public float w;

    public Quaternion() {
      x = 0;
      y = 0;
      z = 0;
      w = 1;
    }

    public Quaternion(float x, float y, float z, float w) {
      this.x = x;
      this.y = y;
      this.z = z;
      this.w = w;
    }

    public Quaternion(Vec3 euler_angles) {
      Vec3 c = Vec3.Cos(euler_angles * 0.5f);
      Vec3 s = Vec3.Sin(euler_angles * 0.5f);

			x = s.x * c.y * c.z - c.x * s.y * s.z;
			y = c.x * s.y * c.z + s.x * c.y * s.z;
			z = c.x * c.y * s.z - s.x * s.y * c.z;
			w = c.x * c.y * c.z + s.x * s.y * s.z;
    }

    public static Vec3 operator*(Quaternion q1 , Vec3 v) {
      Vec3 qv = new Vec3(q1.x, q1.y, q1.z);
      Vec3 uv = Vec3.Cross(qv, v);
      Vec3 uuv = Vec3.Cross(qv, uv);
      return v = ((uv *q1.w) + uuv) * 2.0f;
    }

    public static Quaternion operator*(Quaternion q1 , Quaternion q2) {
      return new Quaternion(
        q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y,
        q1.w * q2.y + q1.y * q2.w + q1.z * q2.x - q1.x * q2.z,
        q1.w * q2.z + q1.z * q2.w + q1.x * q2.y - q1.y * q2.x,
        q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z
      );
    }

    public override int GetHashCode() => (w , x , y , z).GetHashCode();

    public override bool Equals(object obj) => obj is Quaternion q && Equals(q);

    public bool Equals(Quaternion q) => w == q.w && x == q.x && y == q.y && z == q.z;

    public static bool operator==(Quaternion q1 , Quaternion q2) => q1.Equals(q2);
    public static bool operator!=(Quaternion q1 , Quaternion q2) => !q1.Equals(q2);

    public float Length {
      get {
        return (float)Math.Sqrt(LengthSquared);
      }
    }

    public float LengthSquared {
      get {
        return x * x + y * y + z * z + w * w;
      }
    }

    public Vec3 EulerAngles {
      get {
        float roll = Mathf.Atan2(2.0f * (x * y + w * z), w * w + x * x - y * y - z * z);
        float ny = 2.0f * (y * z + w * x);
        float nx = w * w - x * x - y * y + z * z;

        float pitch = 0.0f;

        if (Vec2.EpsilonEquals(new Vec2(nx, ny), Vec2.zero))
          pitch = 2.0f * Mathf.Atan2(x, w);
        else
          pitch = Mathf.Atan2(y, x);

        float yaw = Mathf.Asin(Mathf.Clamp(-2.0f * (x * z - w * y), -1.0f, 1.0f));

        return new Vec3(pitch, yaw, roll);
      }
    }
  }

}