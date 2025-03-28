using System;

namespace Other {

  public class PhysicsObject : Component {
    public PhysicsObject() : base(null) {}

    public PhysicsObject(OtherObject obj) : base(obj) {}
  }

}