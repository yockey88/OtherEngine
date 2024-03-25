using System;

namespace enginey {

  public static class Mathf {
    public const float epsilon = 0.00001f;
    public const float half_pi = 1.57079632679489661923f;
    public const float pi = 3.14159265358979323846f;
    public const float tau = 6.28318530717958647692f;
    public const float deg2rad = 0.01745329251994329577f;
    public const float rad2deg = 57.2957795130823208768f;

    public static float Sin(float angle) => (float)Math.Sin(angle);
    public static float Cos(float angle) => (float)Math.Cos(angle);
    public static float Tan(float angle) => (float)Math.Tan(angle);
    public static float Asin(float angle) => (float)Math.Asin(angle);
    public static float Acos(float angle) => (float)Math.Acos(angle);
    public static float Atan(float angle) => (float)Math.Atan(angle);
    public static float Atan2(float a, float b) => (float)Math.Atan2(a, b);
    public static float Sqrt(float value) => (float)Math.Sqrt(value);
    public static float Abs(float value) => Math.Abs(value);
    public static int Abs(int value) => Math.Abs(value);
    public static float Pow(float value, float power) => (float)Math.Pow(value, power);
    public static float Min(float a, float b) => a < b ? a : b;
    public static float Max(float a, float b) => a > b ? a : b;

    public static float Clamp(float value, float min, float max) {
      if (value < min)
        return min;
      return value > max ? max : value;
    }   

    // public static Vec3 Abs(Vec3 vec) => new Vec3(Abs(vec.x), Abs(vec.y), Abs(vec.z));

    // public static float Lerp(float p1 , float p2 , float t) => Interpolate(p1, p2, t);
    // public static Vec3 Lerp(Vec3 p1 , Vec3 p2 , flaot t) => Interpolate(p1, p2, t);

    public static float Floor(float value) => (float)Math.Floor(value);
    public static float Ceil(float value) => (float)Math.Ceiling(value);
    public static float Modulo(float value, float mod) => value - (Floor(value / mod) * mod);
    public static float Distance(float p1 , float p2) => Abs(p1 - p2);
    public static int CeilToInt(float value) => (int)Ceil(value);
    public static int FloorToInt(float value) => (int)Floor(value);
  }

}
