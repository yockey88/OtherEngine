using System;

namespace Other {

  public abstract class Component {

    public Component() {
    }
    
    public Component(OtherObject obj) {
      this.obj = obj;
    }

    private OtherObject obj;

    public OtherObject Object { 
      get => obj;
      set => obj = value;
    }
  }

  public class Tag : Component {
    private ulong id;

    // public String Name {
    //   get => Scene.GetName(id);
    // }

    public ulong ID {
      get => id;
      set => id = value;
    }

    public Tag(OtherObject obj) : base(obj) {
      id = obj.ObjectID;
    }
  }

}
