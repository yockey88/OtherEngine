using System;

namespace Other {

  public class TestScript2 : OtherObject {
    UInt32 accum = 0;

    public override void Update(float dt) {
      Transform transform = GetComponent<Transform>();
      if (transform == null) {
        Console.WriteLine(" > Transform is null");
        return;
      }

      transform.Rotate(0.1f , new Vec3(1.0f , 1.0f , 1.0f));


      // accum += (UInt32)dt;
      // if (accum != 0 && accum % 100.0f < 1.0f) {
      //   transform.Position += new Vec3(dt);
      //   Logger.WriteDebug($"Position updated : {accum}");
      //   accum = 0;
      // }
    }
  }

}