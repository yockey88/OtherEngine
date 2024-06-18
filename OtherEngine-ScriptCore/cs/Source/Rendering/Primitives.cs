using System.Runtime.InteropServices;

namespace Other {

  [StructLayout(LayoutKind.Sequential)]
  public struct Line {
    public Vec3 start;
    public Vec3 end;

    public Line(Vec3 start, Vec3 end) {
      this.start = start;
      this.end = end;
    }

    public void Translate(Vec3 translation) {
      start += translation;
      end += translation;
    }

    public void Scale(float scale) {
      start *= scale;
      end *= scale;
    }

    public void Scale(Vec3 scale) {
      start *= scale;
      end *= scale;
    }
  }

  [StructLayout(LayoutKind.Sequential)]
  public struct Triangle {
    private Vec3 a;
    private Vec3 b;
    private Vec3 c;

    public Triangle(Vec3 a, Vec3 b, Vec3 c) {
      this.a = a;
      this.b = b;
      this.c = c;
    }

    public void Translate(Vec3 translation) {
      a += translation;
      b += translation;
      c += translation;
    }

    public void Scale(float scale) {
      a *= scale;
      b *= scale;
      c *= scale;
    }

    public void Scale(Vec3 scale) {
      a *= scale;
      b *= scale;
      c *= scale;
    }
  }

  [StructLayout(LayoutKind.Sequential)]
  public struct Rect {
    public Vec3 position;
    public Vec2 half_extents;

    public Rect(Vec3 position, Vec2 half_extents) {
      this.position = position;
      this.half_extents = half_extents;
    }

    public void Translate(Vec3 translation) {
      position += translation;
    }

    public void Scale(float scale) {
      half_extents *= scale;
    }

    public void Scale(Vec2 scale) {
      half_extents *= scale;
    }
  }

  [StructLayout(LayoutKind.Sequential)]
  public struct Cube {
    public Vec3 position;
    public Vec3 half_extents;

    public Cube(Vec3 position, Vec3 half_extents) {
      this.position = position;
      this.half_extents = half_extents;
    }
  }

  [StructLayout(LayoutKind.Sequential)]
  public struct Circle {
    public Vec3 position;
    public float radius;

    public Circle(Vec3 position, float radius) {
      this.position = position;
      this.radius = radius;
    }
  }

  [StructLayout(LayoutKind.Sequential)]
  public struct Sphere {
    public Vec3 position;
    public float radius;

    public Sphere(Vec3 position, float radius) {
      this.position = position;
      this.radius = radius;
    }
  }

  [StructLayout(LayoutKind.Sequential)]
  public struct Capsule {
    public Vec3 position;
    public Vec3 normal;
    public float radius;
    public float height;

    public Capsule(Vec3 position, Vec3 normal, float radius, float height) {
      this.position = position;
      this.normal = normal;
      this.radius = radius;
      this.height = height;
    }
  }

  [StructLayout(LayoutKind.Sequential)]
  public struct Cylinder {
    public Vec3 position;
    public Vec3 normal;
    public float radius;
    public float height;

    public Cylinder(Vec3 position, Vec3 normal, float radius, float height) {
      this.position = position;
      this.normal = normal;
      this.radius = radius;
      this.height = height;
    }
  }

}