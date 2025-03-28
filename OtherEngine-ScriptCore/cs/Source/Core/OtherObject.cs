using System;
using System.Collections.Generic;
using System.Diagnostics.Contracts;
using DotOther.Managed;
using DotOther.Managed.Interop;

namespace Other {

  public struct BehaviorFlags {
    public bool parent_loaded;
    public bool children_loaded;

    public BehaviorFlags(bool parent_flag, bool children_flag) {
      parent_loaded = parent_flag;
      children_loaded = children_flag;
    }
  }


  public class OtherObject : OtherBehavior {
    private Dictionary<Type , Component> components = new Dictionary<Type , Component>();
    private BehaviorFlags flags = new BehaviorFlags(false , false);

    internal static unsafe delegate*<IntPtr , NString> GetName;
    internal static unsafe delegate*<IntPtr , NString , void> SetName;

    internal static unsafe delegate*<IntPtr , UInt64> GetInternalID;

    internal static unsafe delegate*<IntPtr , UInt32> GetEntityID;

    internal static unsafe delegate*<IntPtr , ReflectionType , bool> NativeHasComponent;
    internal static unsafe delegate*<IntPtr , ReflectionType , void> NativeCreateComponent;
    internal static unsafe delegate*<IntPtr , ReflectionType , void> NativeRemoveComponent;

    
    private UInt32 entity_id = 0;

    public UInt32 EntityID {
      get { return entity_id; }
      internal set { entity_id = value; }
    }

    public string Name {
      get  {
        unsafe {
          NString native_name = GetName(NativeHandle);
          return native_name.ToString();
        }
      }
      set {
        unsafe {
          SetName(NativeHandle , value);
        }
      }
    }

    public UUID InternalId {
      get {
        unsafe {
          return OtherObject.GetInternalID(NativeHandle);
        }
      }
    }

    public UInt32 EntityId {
      get {
        unsafe {
          return OtherObject.GetEntityID(NativeHandle);
        }
      }
    }

    public OtherObject() {
      ObjectID = 0;
      NativeHandle = IntPtr.Zero;
    }
    public OtherObject(IntPtr native_handle) {
      NativeHandle = native_handle;
      ObjectRegistry.Register(this);
    }

    public OtherObject(IntPtr native_handle, UInt64 object_id, UInt32 scene_id) {
      NativeHandle = native_handle;
      ObjectID = object_id;
      EntityID = scene_id;
      ObjectRegistry.Register(this);
    }


    ~OtherObject() {
      ObjectRegistry.Unregister(this);
    }

    public override OtherBehavior Parent {
      get {
        if (parent == null) {
          flags.parent_loaded = true;
        }
        return parent;
      }
      set { parent = value; }
    }

    public override List<OtherBehavior> Children {
      get { 
        if ((children == null || children.Count == 0) && !flags.children_loaded) {
          for (int i = 0; i < children.Count; i++) {
            children[i].Parent = this;
          }
          flags.children_loaded = true;
        }
        return children; 
      }
      set { children = value; }
    }
    
    // public override void NativeInitialize() {
    //   Scene.AddActiveScript(ObjectID);
    // }

    public override void NativeStart() {
    }

    public T CreateComponent<T>() where T : Component , new() {
      if (HasComponent<T>()) {
        return GetComponent<T>();
      }
      
      Type comp_type = typeof(T);
      unsafe {
        NativeCreateComponent(NativeHandle , comp_type);
      }

      T comp = new T();
      comp.Object = this;
      components.Add(comp_type , comp);
      
      return comp;
    }  
    
    public bool HasComponent<T>() where T : Component {
      unsafe {
        return NativeHasComponent(NativeHandle, typeof(T));
      }
    }

    public T GetComponent<T>() where T : Component , new() {
      Type comp_type = typeof(T);

      if (!HasComponent<T>()) {
        return null;
      }

      if (components.TryGetValue(comp_type , out Component comp)) {
        return (T)comp;
      }

      comp = new T();
      comp.Object = this;
      components.Add(comp_type , comp);
      return (T)comp;
    }

    public void RemoveComponent<T>() where T : Component {
      if (!HasComponent<T>()) {
        return;
      }

      Type comp_type = typeof(T);
      components.Remove(comp_type);

      unsafe {
        NativeRemoveComponent(NativeHandle , comp_type);
      }
      return;
    }

    public void BeginContact(UInt64 other) {
      if (!Scene.IsActive()) {
        return;
      }

      OtherObject other_object = ObjectRegistry.LookUp(other);
      if (other_object == null) {
        other_object = Scene.GetObject(other);
        if (other_object == null) {
          return;
        }
      }
      
      BeginContact(other_object);
    }

    public void HandleCollisionPoint(IntPtr collision_data_native) {
      if (!Scene.IsActive()) {
        return;
      }


      CollisionPointData? collision_data_raw = null;
      try {
        collision_data_raw = DotOther.Managed.Interop.DotOtherMarshal.MarshalPointer<CollisionPointData>(collision_data_native);
      } catch (Exception e) {
        Logger.WriteError($"Failed to marshal collision data: {e.Message}");
        return;
      }

      if (collision_data_raw == null) {
        return;
      }

      CollisionPointData collision_data = collision_data_raw.Value;
      OtherObject other_object = ObjectRegistry.LookUp(collision_data.Object);
      if (other_object == null) {
        other_object = Scene.GetObject(collision_data.Object);
        if (other_object == null) {
          return;
        }
      }
      
      HandleContact(other_object, collision_data.Point);
    }

    public void EndContact(UInt64 other) {
      if (!Scene.IsActive()) {
        return;
      }

      OtherObject other_object = ObjectRegistry.LookUp(other);
      if (other_object == null) {
        other_object = Scene.GetObject(other);
        if (other_object == null) {
          return;
        }
      }
      
      EndContact(other_object);
    }

    protected virtual void BeginContact(OtherObject other) {}

    protected virtual void HandleContact(OtherObject other, Vec3 point) {}

    protected virtual void EndContact(OtherObject other) {}

  }

}
