using System;
using System.Collections.Generic;

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

    public ulong Id => ObjectID;
    public string Name => Scene.GetName(ObjectID);

    // private Transform object_transform;

    public OtherObject() {
    }

    public override OtherBehavior Parent {
      get {
        if (parent == null) {
          parent = Scene.GetParent(ObjectID);
          flags.parent_loaded = true;
        }
        return parent;
      }
      set { parent = value; }
    }

    public override List<OtherBehavior> Children {
      get { 
        if ((children == null || children.Count == 0) && !flags.children_loaded) {
          children = Scene.GetChildren(ObjectID);
          for (int i = 0; i < children.Count; i++) {
            children[i].Parent = this;
          }
          flags.children_loaded = true;
        }
        return children; 
      }
      set { children = value; }
    }

    public override void OnBehaviorLoad() {
      Scene.AddObject(this);
      // object_transform = new Transform(this);
    }

    public override void OnBehaviorUnload() {
      Parent = null;
      if (Children != null) {
        Children.Clear();
        Children = null;
      }
    }

    /// might need to call into the native side to get/set the transform here
    // public Transform Transform {
    //   get { return object_transform; }
    //   set { object_transform = value; }
    // }

    public T CreateComponent<T>() where T : Component , new() {
      if (HasComponent<T>()) {
        return GetComponent<T>();
      }
      
      Type comp_type = typeof(T);
      Scene.AddComponent(Id, comp_type);

      T comp = new T();
      comp.Object = this;
      components.Add(comp_type , comp);
      
      return comp;
    }  
    
    public bool HasComponent<T>() where T : Component => Scene.HasComponent(Id, typeof(T));

    public T GetComponent<T>() where T : Component , new() {
      Type comp_type = typeof(T);

      if (!HasComponent<T>()) {
        return null;
      }

      try {
        components.TryGetValue(comp_type , out Component comp);
        return comp as T;
      } catch (Exception e) {
        Logger.WriteError(e.Message);
        return null;
      }
    }

    public void RemoveComponent<T>() where T : Component {
      if (!HasComponent<T>()) {
        return;
      }

      Type comp_type = typeof(T);
      Scene.RemoveComponent(Id, comp_type);
      components.Remove(comp_type);
      
      return;
    }

    
  }

}
