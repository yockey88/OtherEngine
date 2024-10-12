using System;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;

using DotOther;
using DotOther.Managed;
using DotOther.Managed.Interop;

namespace DotOther.Tests {

  public class Mod1 : OtherObject {
    private Int32 my_num;

    private Int32 my_int;

    public Int32 MyInt {
      get {
        Console.WriteLine($"Mod1.get_MyInt: {my_int}");
        return my_int;
      }
      set {
        Console.WriteLine($"Mod1.set_MyInt: {value}");
        my_int = value;
      }
    }

    public Int32 MyNum {
      get => my_num;
      set => my_num = value;
    }

    public float number = 0.0f;

    public Mod1() {
      Console.WriteLine($" Mod1 Asm Name : {this.GetType().AssemblyQualifiedName}");
      my_num = 0;
    }

    public void Test() {
      Console.WriteLine("Mod1.Test");
    }

    public void Test(Int32 num) {
      Console.WriteLine("Mod1.Test: " + num);
    }

    public void TestInternalCall() {
      Console.WriteLine("Mod1.TestInternalCall");
      unsafe {
        Vec3 vec = DotOtherMarshal.MarshalPointer<Vec3>(GetVec3());
        Console.WriteLine($" > MyVec3 : {vec.ToString()}");
      }
    }

    private static unsafe delegate*<IntPtr> GetVec3;
  }

  [StructLayout(LayoutKind.Sequential , Size = 12)]
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

    public Vec3(Vec3 vec) {
      x = vec.x;
      y = vec.y;
      z = vec.z;
    }
    public override bool Equals(object obj) => obj is Vec3 other && Equals(other);
    public bool Equals(Vec3 right) => x == right.x && y == right.y && z == right.z;
    public override int GetHashCode() => (x , y , z).GetHashCode();
    public static bool operator ==(Vec3 left, Vec3 right) => left.Equals(right);
    public static bool operator !=(Vec3 left, Vec3 right) => !(left == right);
    public override string ToString() => $"Vec3[{x} , {y} , {z}]";
  }

}