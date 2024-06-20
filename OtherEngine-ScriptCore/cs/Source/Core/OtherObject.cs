using System;
using System.Runtime.Serialization;

namespace Other {

  public class OtherObject : IEquatable<OtherObject> {

    private UInt64 object_id;

    public OtherObject() {
      object_id = 0;
    }

    public UInt64 ObjectID {
      get { return object_id; }
      set {
        object_id = value;  
        Logger.WriteDebug($"ObjectID: {object_id}");
      }
    }

    public virtual void OnInitialize() {}

    public virtual void OnStart() {}

    public virtual void Update(float dt) {}

    public virtual void RenderObject() {}

    public virtual void RenderUI() {}

    public virtual void OnStop() {}

    public virtual void OnShutdown() {}

    public override bool Equals(object obj) => obj is OtherObject other && Equals(other);

    public bool Equals(OtherObject other) {
      if (other is null) {
        return false;
      }
      
      if (ReferenceEquals(this , other)) {
        return true;
      }

      return object_id == other.object_id;
    }

    public override int GetHashCode() => (int)object_id; 

    public static bool operator ==(OtherObject left , OtherObject right) => left is null ? right is null : left.Equals(right);
    public static bool operator !=(OtherObject left , OtherObject right) => !(left == right);
  }

}