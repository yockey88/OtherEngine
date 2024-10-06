using System;
using System.Collections.Generic;

namespace Other {

  public struct ObjectKey : IEquatable<ObjectKey> {
    public UInt64 handle;
    public UInt32 entity_id;
    public IntPtr native_handle;

    public ObjectKey(UInt64 handle, UInt32 entity_id, IntPtr native_handle) {
      this.handle = handle;
      this.entity_id = entity_id;
      this.native_handle = native_handle;
    }

    public bool Equals(ObjectKey other) => native_handle == other.native_handle && handle == other.handle && entity_id == other.entity_id;
    public override bool Equals(object obj) => obj is ObjectKey other && Equals(other);

    public override int GetHashCode() => HashCode.Combine(handle , entity_id , native_handle);

    public static bool operator==(ObjectKey left, ObjectKey right) => left.Equals(right);
    public static bool operator!=(ObjectKey left, ObjectKey right) => !(left == right);
  }

  class ObjectRegistry {
    private static Dictionary<ObjectKey, OtherObject> behaviors = new Dictionary<ObjectKey, OtherObject>();
    public static void Register(OtherObject behavior) {
      unsafe {
        if (behavior.NativeHandle == IntPtr.Zero) {
          Logger.WriteError("Registering object with invalid handle");
          return;
        }

        ObjectKey key = new ObjectKey(behavior.ObjectID, behavior.EntityID, behavior.NativeHandle);
        if (behaviors.ContainsKey(key)) {
          return;
        }

        behaviors.Add(key, behavior);
      }
    }

    public static OtherObject GetBehavior(UInt64 handle, UInt32 entity_id, IntPtr native_handle) {
      ObjectKey key = new ObjectKey(handle, entity_id, native_handle);
      if (behaviors.ContainsKey(key)) {
        return behaviors[key];
      }
      return null;
    }

#nullable disable
    public static OtherObject? TryGetObject(UInt64? handle, UInt32? entity_id, IntPtr? native_handle) {
      Logger.WriteTrace($"Searching for object with handle: {handle}, entity_id: {entity_id}, native_handle: {native_handle}");
      List<ObjectKey> keys = new List<ObjectKey>();
      foreach (ObjectKey key in keys) {
        if (native_handle != null && key.native_handle == native_handle) {
          return behaviors[key];
        }

        bool handle_match = handle == null || key.handle == handle;
        bool entity_id_match = entity_id == null || key.entity_id == entity_id;
        bool native_handle_match = native_handle == null || key.native_handle == native_handle;
        if (handle_match || entity_id_match || native_handle_match) {
          keys.Add(key);
        }
      }

      if (keys.Count == 0) {
        return null;
      }

      if (keys.Count > 1) {
        Logger.WriteWarning("Multiple objects found with the same handle");
      }

      return behaviors[keys[0]];
    }
#nullable disable

    public static void Unregister(OtherBehavior behavior) {
      unsafe {
        if (behavior.NativeHandle == IntPtr.Zero) {
          Logger.WriteError("Unregistering object with invalid handle");
          return;
        }
        behaviors.Remove(new ObjectKey(behavior.ObjectID, behavior.EntityID, behavior.NativeHandle));
      }
    }
  }

}