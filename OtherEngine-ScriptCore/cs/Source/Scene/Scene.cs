using System;
using System.Collections.Generic;

namespace Other {

  public class Scene : OtherObject {
    private static Dictionary<ulong , OtherObject> objects = new Dictionary<ulong , OtherObject>();

    public override void OnBehaviorLoad() {
      Console.WriteLine("Scene OnBehaviorLoad");
    }

    public override void OnBehaviorUnload() {
      Console.WriteLine("Scene OnBehaviorUnload");
    }
  }

}