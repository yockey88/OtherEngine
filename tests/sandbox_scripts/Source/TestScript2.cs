using System;

namespace Other {

  public class TestScript2 : OtherObject {
    UInt32 accum = 0;

    public override void Update(float dt) {}

    public override void OnContact(OtherObject other) {}

    public void PrintAccum() {
      Console.WriteLine(accum);
    }
  }

}