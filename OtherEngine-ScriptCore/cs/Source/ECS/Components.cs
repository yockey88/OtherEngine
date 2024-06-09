using System;

namespace Other {

  public abstract class Component {
    public Entity Entity { get; internal set; }
  }

  public class Tag : Component {
    public String Name;
    public ulong Id;
  }

  public class Transform : Component {}

  public class Relationship : Component {}

  public class Script : Component {}

  public class Mesh : Component {
    public ulong asset_handle;
  }

}
