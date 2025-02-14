using System;

namespace Other {

  public class TestScript2 : OtherObject {
    UInt32 accum = 0;

    public override void Update(float dt) {}

    public void PrintAccum() {
      Console.WriteLine(accum);
    }
  }

}