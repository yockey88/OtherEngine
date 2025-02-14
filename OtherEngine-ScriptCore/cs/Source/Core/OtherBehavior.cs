using System;
using System.Collections.Generic;

namespace Other {

  /// <summary>
  ///   OtherBehavior is the base class for all behaviors in the engine.
  ///   It provides the basic structure for behaviors to be attached to native objects as long as those objects have two things
  ///     1. A unique ID (64 bits and unique across all objects of all data types registered with Other during the application's lifetime)
  ///     2. A native handle (a pointer to the native object)
  ///   Types of behaviors include:
  ///   - OtherObject: a scene object inside of an OtherEngine scene
  ///   - EditorPanel: a panel that can be attached to the OtherEngine editor
  ///   - etc.....
  ///   
  ///   OtherBehavior also provides the ability to attach Behaviors to other Behaviors, or attach other Behaviors to a single Behavior, 
  ///      creating a tree-like structure of Behaviors
  ///      
  ///   OtherBehaviors are all enabled by default, this can be overriden on a per-behavior basis by setting 'Enabled' to false in the behavior's constructor
  /// </summary>
  public abstract class OtherBehavior : IEquatable<OtherBehavior> {
    private bool enabled = true;
    private UUID object_id = 0;
    private IntPtr native_handle;

#nullable enable
    protected OtherBehavior? parent;

    protected List<OtherBehavior> children = new List<OtherBehavior>();

    public virtual OtherBehavior Parent {
      get { return parent!; }
      set { parent = value; }
    }
  
    public virtual List<OtherBehavior> Children {
      get { return children; }
      set { children = value; }
    }
#nullable disable

    public bool Enabled {
      get { return enabled; }
      set { enabled = value; }
    }

    public UInt64 ObjectID {
      get { return object_id; }
      internal set { object_id = value; }
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
