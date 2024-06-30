using System;
using System.Collections.Generic;

namespace Other {

  public class OtherObject : OtherBehavior {
    private Dictionary<Type , Component> components = new Dictionary<Type , Component>();
    public ulong Id => ObjectID;
    public string Name => Scene.GetName(ObjectID);

    private Transform object_transform;

    public OtherObject() {
      object_transform = new Transform(this);
    }

    public override OtherBehavior Parent {
      get {
        parent = Scene.GetParent(ObjectID);
        return parent;
      }
      set { parent = value; }
    }

    public override List<OtherBehavior> Children {
      get { 
        return children; 
      }
      set { children = value; }
    }

    public override void OnBehaviorLoad() {
      /// add object to the scene
      Scene.AddObject(this);

      /// initialize data from native side
      object_transform = new Transform(this);
      Parent = Scene.GetParent(ObjectID);
      Children = Scene.GetChildren(ObjectID);
      for (int i = 0; i < Children.Count; i++) {
        Children[i].Parent = this;
      }
    }

    public override void OnBehaviorUnload() {
      Parent = null;
      if (Children != null) {
        Children.Clear();
        Children = null;
      }
    }

    /// might need to call into the native side to get/set the transform here
    public Transform Transform {
      get { return object_transform; }
      set { object_transform = value; }
    }

    public T CreateComponent<T>() where T : Component, new() {
      if (HasComponent<T>()) {
        return GetComponent<T>();
      }
      
      Type comp_type = typeof(T);
      Scene.AddComponent(Id, comp_type);

      T comp = new T { Object = this };
      components.Add(comp_type , comp);
      
      return comp;
    }  
    
    public bool HasComponent<T>() where T : Component => Scene.HasComponent(Id, typeof(T));

    public T GetComponent<T>() where T : Component , new() {
      Type comp_type = typeof(T);

      if (!HasComponent<T>()) {
        return null;
      }

      components.TryGetValue(comp_type , out Component comp);

      if (comp == null) {
        return CreateComponent<T>();
      }

      return comp as T;
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