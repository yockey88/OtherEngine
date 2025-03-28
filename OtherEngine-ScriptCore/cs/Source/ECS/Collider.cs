using System;

namespace Other {

  public class Collider : Component {
    public Collider() : base(null) {}

    public Collider(OtherObject obj) : base(obj) {}
  }

}