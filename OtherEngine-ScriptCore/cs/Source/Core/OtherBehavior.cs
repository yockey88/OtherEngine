using System;
using System.Collections.Generic;

namespace Other {

  public abstract class OtherBehavior : IEquatable<OtherBehavior> {
    private bool enabled = true;

    private UInt64 object_id;

    protected OtherBehavior parent;

    protected List<OtherBehavior> children = new List<OtherBehavior>();

    public abstract OtherBehavior Parent {
      get;
      set;
    }

    public abstract List<OtherBehavior> Children {
      get;
      set;
    }

    public bool Enabled {
      get { return enabled; }
      set { enabled = value; }
    }

    public UInt64 ObjectID {
      get { return object_id; }
      set { object_id = value; }
    }
    
    public OtherBehavior() {
      object_id = 0;
    }

    public void Enable() {
      enabled = true;
    }

    public void Disable() {
      enabled = false;
    }

    public bool IsEnabled() {
      return enabled;
    }

    public abstract void OnBehaviorLoad();
    public abstract void OnBehaviorUnload();

    public virtual void OnInitialize() {}

    public virtual void OnStart() {}

    public virtual void Update(float dt) {}
    public virtual void LateUpdate(float dt) {}

    public virtual void RenderObject() {}

    public virtual void RenderUI() {}

    public virtual void OnStop() {}

    public virtual void OnShutdown() {}

    public override bool Equals(object obj) => obj is OtherBehavior other && Equals(other);

    public bool Equals(OtherBehavior other) {
      if (other is null) {
        return false;
      }
      
      if (ReferenceEquals(this , other)) {
        return true;
      }

      return object_id == other.object_id;
    }

    public override int GetHashCode() => (int)object_id; 

    public static bool operator ==(OtherBehavior left , OtherBehavior right) => left is null ? right is null : left.Equals(right);
    public static bool operator !=(OtherBehavior left , OtherBehavior right) => !(left == right);
  }

}
