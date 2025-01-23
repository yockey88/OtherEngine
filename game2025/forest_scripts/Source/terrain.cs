using System;

using Other;

namespace Forest {

  class Terrain : OtherObject {
    public override void Update(float dt) {
    }

    public override void OnContact(OtherObject other) {
      Console.WriteLine("Terrain OnContact");
    }
  }

}