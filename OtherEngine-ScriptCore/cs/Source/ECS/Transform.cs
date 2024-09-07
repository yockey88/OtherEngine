using System;
using System.Runtime.InteropServices;
using System.Security.Policy;

namespace Other {

  [StructLayout(LayoutKind.Sequential)]
  public class Transform : Component {
    public Transform() { }
    public Transform(OtherObject obj) : base(obj) { }

    public Vec3 scale {
      get {
        Scene.GetScale(Object.ObjectID , out Vec3 scale);
        return scale;
      }
      set => Scene.SetScale(Object.ObjectID , ref value);
    }

    public Vec3 half_extents {
      get {
        Vec3 scale = this.scale;
        scale.x *= 0.5f;
        scale.y *= 0.5f; 
        return scale;
      }
    }

    public Vec3 position {
      get {
        Scene.GetPosition(Object.ObjectID , out Vec3 position);
        return position;
      }
      set => Scene.SetPosition(Object.ObjectID , ref value);
    }

    public Vec3 rotation {
      get {
        Scene.GetRotation(Object.ObjectID , out Vec3 rotation);
        return rotation;
      }
      set => Scene.SetRotation(Object.ObjectID , ref value);
    }

    public void Rotate(float angle , Vec3 axis) {
      Scene.RotateObject(Object.ObjectID , angle , ref axis);
    }
  }
    
}