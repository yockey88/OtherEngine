using System.Collections.Generic;

namespace Other {

  public class Editor : OtherBehavior {
    public override OtherBehavior Parent {
      get;
      set;
    }

    public override List<OtherBehavior> Children {
      get;
      set;
    }

    public override void OnBehaviorLoad() {}
    public override void OnBehaviorUnload() {}
  }

}
