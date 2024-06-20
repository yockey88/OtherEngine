using System;
using System.Collections.Generic;

namespace Other {

  /// <summary>
  ///  Base class for all game objects. Must be inherited from to be used.
  ///  Contains methods for creating, getting, and removing components from the Engine internal 'Entity' object.
  ///  Also contains a reference to the parent object.
  /// </summary>
   public class OtherBehavior : OtherObject {
    private Dictionary<Type , Component> components = new Dictionary<Type , Component>();

    public string Name => GetComponent<Tag>().Name;
    public ulong Id => GetComponent<Tag>().Id;
    
    protected Transform transform;

    private OtherObject parent;

    public OtherObject Parent {
      get {
        ulong id = 0; // Engine.GetParent(Id);
        if (parent == null || parent.ObjectID != id) {
          parent = false ? // Engine.IsEntityValid(id) ?
            new OtherObject() : null;
        }
        if (parent != null) {
          parent.ObjectID = id;
        }
        return parent;
      }
      set {}
    }

    public T CreateComponent<T>() where T : Component, new() {
      if (HasComponent<T>()) {
        return GetComponent<T>();
      }
      
      Type comp_type = typeof(T);
      // Engine.EntityAddComponent(Id, comp_type);

      T comp = new T { Object = this };
      components.Add(comp_type , comp);
      
      return comp;
    }  
    
    public bool HasComponent<T>() where T : Component => false; // Engine.EntityHasComponent(Id, typeof(T));

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

    public bool RemoveComponent<T>() where T : Component {
      Type comp_type = typeof(T);
      bool remvd = false; // Engine.EntityRemoveComponent(Id, comp_type);
      
      if (remvd) {
          components.Remove(comp_type);
      }
      
      return remvd;
    }
  }

}
