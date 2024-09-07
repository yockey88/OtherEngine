using System;

namespace Other {

  public class UUID : IEquatable<UUID> {
    UInt64 id;

    public UUID() {
      id = 0;
    }

    public UUID(UInt64 id) {
      this.id = id;
    }

    public bool Equals(UUID other) {
      return id == other.id;
    }
    public override bool Equals(object obj) {
      return obj is UUID other && Equals(other);
    }
    public override int GetHashCode() => id.GetHashCode();
    
    public static bool operator ==(UUID left, UUID right) => left.Equals(right);
    public static bool operator !=(UUID left, UUID right) => !(left == right);
    public override string ToString() => $"UUID[{id}]";

    public static implicit operator UInt64(UUID uuid) => uuid.id;
    public static implicit operator UUID(UInt64 id) => new UUID(id);
  }

}