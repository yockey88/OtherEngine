using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace Other {

  public abstract class OtherBehavior : IEquatable<OtherBehavior> {
    private bool enabled = true;

    /// <summary>
    ///  ID of the engine object using this behavior
    /// </summary>
    private UUID object_id = 0;
    private UInt32 entity_id = 0;
    private IntPtr native_handle;

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
      internal set { object_id = value; }
    }

    public UInt32 EntityID {
      get { return entity_id; }
      internal set { entity_id = value; }
    }

    public IntPtr NativeHandle {
      get { return native_handle; }
      internal set  { unsafe { native_handle = value; } }
    }
    
    public OtherBehavior() {
      object_id = 0;
      native_handle = IntPtr.Zero;
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

    public virtual void OnBehaviorLoad() {}
    public virtual void OnBehaviorUnload() {}

    public virtual void NativeInitialize() {}
    public virtual void OnInitialize() {}

    public virtual void OnShutdown() {}
    public virtual void NativeShutdown() {}

    public virtual void NativeStart() {}
    public virtual void OnStart() {}

    public virtual void OnStop() {}
    public virtual void NativeStop() {}

    public virtual void EarlyUpdate(float dt) {}
    public virtual void Update(float dt) {}
    public virtual void LateUpdate(float dt) {}

    public virtual void Render() {}
    public virtual void RenderUI() {}

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

    public override int GetHashCode() => object_id.GetHashCode(); 

    public static bool operator ==(OtherBehavior left , OtherBehavior right) => left is null ? right is null : left.Equals(right);
    public static bool operator !=(OtherBehavior left , OtherBehavior right) => !(left == right);
  }

}
