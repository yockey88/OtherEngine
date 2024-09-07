using System.Runtime.InteropServices;

namespace Other {

  [StructLayout(LayoutKind.Sequential, Pack = 16)]
  public class Material {
    Vec4 color;
    float shininess;

    public Material() {
      color = new Vec4(1.0f, 1.0f, 1.0f, 1.0f);
      shininess = 32.0f;
    }

    public Material(Vec4 color, float shininess) {
      this.color = color;
      this.shininess = shininess;
    }

    public Material(float r, float g, float b, float a, float shininess) {
      color = new Vec4(r, g, b, a);
      this.shininess = shininess;
    }

    public Vec4 Color {
      get => color;
      set => color = value;
    }

    public float Shininess {
      get => shininess;
      set => shininess = value;
    }
  }

}