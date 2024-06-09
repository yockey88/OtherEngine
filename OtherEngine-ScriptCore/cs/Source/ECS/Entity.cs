using System;
using System.Collections.Generic;

namespace Other {

  public class Entity : IEquatable<Entity> {
    private Dictionary<Type , Component> components = new Dictionary<Type , Component>();

    public string Name => GetComponent<Tag>().Name;
    public ulong Id => GetComponent<Tag>().Id;
    
    protected Transform transform;

    private Entity parent;

    public Entity Parent {
      get {
        ulong id = 0; // Engine.GetParent(Id);
        if (parent == null || parent.Id != id) {
          parent = false ? // Engine.IsEntityValid(id) ?
            new Entity(id) : null;
        }
        return parent;
      }
      set {}
    }

    protected Entity() {
      GetComponent<Tag>().Id = 0;
      transform = new Transform { Entity = this };
      components.Add(typeof(Transform) , transform);
    }

    public Entity(ulong id) {
      GetComponent<Tag>().Id = id;
      transform = new Transform { Entity = this };
      components.Add(typeof(Transform) , transform);
    }

    public T CreateComponent<T>() where T : Component, new() {
      if (HasComponent<T>()) {
        return GetComponent<T>();
      }
      
      Type comp_type = typeof(T);
      // Engine.EntityAddComponent(Id, comp_type);

      T comp = new T { Entity = this };
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

    public override bool Equals(object obj) => obj is Entity other && Equals(other);

    public bool Equals(Entity other) {
      if (other is null) {
        return false;
      }
      
      if (ReferenceEquals(this , other)) {
        return true;
      }

      return Id == other.Id;
    }

    public override int GetHashCode() => (int)Id; 

    public static bool operator ==(Entity left , Entity right) => left is null ? right is null : left.Equals(right);
    public static bool operator !=(Entity left , Entity right) => !(left == right);
  };

}
