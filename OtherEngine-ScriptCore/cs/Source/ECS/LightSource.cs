using System;
using System.Runtime.InteropServices;

namespace Other {

  public class LightSource : Component {
    [StructLayout(LayoutKind.Sequential , Size = 16)]
    public struct PointLightData {
      public float radius;
      public float constant;
      public float linear;
      public float quadratic;
    }

    public LightSource(OtherObject obj) : base(obj) { }

    internal static unsafe delegate*<IntPtr , Vec4* , void> GetVector;
    internal static unsafe delegate*<IntPtr , Vec4* , void> SetVector;

    static internal unsafe delegate*<IntPtr , Vec4* , void> GetColor;
    static internal unsafe delegate*<IntPtr , Vec4* , void> SetColor;

    static internal unsafe delegate*<IntPtr , Vec4* , void> GetPointLight;
    static internal unsafe delegate*<IntPtr , Vec4* , void> SetPointLight;

    public Vec4 Vector {
      get {
        unsafe {
          Vec4 vector = Vec4.zero;
          GetVector(Object.NativeHandle , &vector);
          return vector;
        }
      }
      set {
        unsafe {
          SetVector(Object.NativeHandle , &value);
        }
      }
    }

    public Vec4 Color {
      get {
        unsafe {
          Vec4 color = Vec4.zero;
          GetColor(Object.NativeHandle , &color);
          return color;
        }
      }
      set {
        unsafe {
          SetColor(Object.NativeHandle , &value);
        }
      }
    }

    private Vec4 PointLight {
      get {
        unsafe {
          Vec4 data = new Vec4();
          GetPointLight(Object.NativeHandle , &data);
          return data;
        }
      }
      set {
        unsafe {
          SetPointLight(Object.NativeHandle , &value);
        }
      }
    }

    public float Radius {
      get => PointLight.x;
      set {
        Vec4 data = PointLight;
        data.x = value;
        PointLight = data;
      }
    }

    public float Constant {
      get => PointLight.y;
      set {
        Vec4 data = PointLight;
        data.y = value;
        PointLight = data;
      }
    }

    public float Linear {
      get => PointLight.z;
      set {
        Vec4 data = PointLight;
        data.z = value;
        PointLight = data;
      }
    }

    public float Quadratic {
      get => PointLight.w;
      set {
        Vec4 data = PointLight;
        data.w = value;
        PointLight = data;
      }
    }
  }

}