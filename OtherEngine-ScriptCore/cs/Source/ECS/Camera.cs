namespace Other {

  public class Camera : Component {
    public Camera() {}

    public Camera(OtherObject obj) : base(obj) {
    }

    public Vec3 Position {
      get {
        Scene.GetPosition(Object.ObjectID , out Vec3 position);
        return position;
      }
      set => Scene.SetPosition(Object.ObjectID , ref value);
    }

    // public float Speed {
    //   get {
    //   //   Scene.GetSpeed(Object.ObjectID , out float speed);
    //   //   return speed;
    //   }
    //   // set => Scene.SetSpeed(Object.ObjectID , ref value);
    // }
  }

}