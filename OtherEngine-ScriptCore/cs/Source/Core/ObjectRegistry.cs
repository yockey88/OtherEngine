using System;
using System.Collections.Generic;
using System.Collections.Immutable;
using System.IO.Compression;
using System.Runtime.ExceptionServices;
using System.Security.AccessControl;

namespace Other {

  class ObjectRegistry {
    private static Dictionary<ObjectKey, OtherObject> objects = new Dictionary<ObjectKey, OtherObject>();
    public static void Register(OtherObject obj) {
      unsafe {
        if (obj.NativeHandle == IntPtr.Zero) {
          Logger.WriteError("Registering object with invalid handle");
          return;
        }

        ObjectKey key = new ObjectKey(obj);
        if (objects.ContainsKey(key)) {
          return;
        }

        objects.Add(key, obj);
      }
    }

    public static OtherObject GetObject(UInt64 handle, UInt32 entity_id, IntPtr native_handle) {
      ObjectKey key = new ObjectKey(handle, entity_id, native_handle);
      if (objects.ContainsKey(key)) {
        return objects[key];
      }
      return null;
    }

#nullable disable
    public static OtherObject? TryGetObject(UInt64? handle, UInt32? entity_id, IntPtr? native_handle) {
      try {
        Int64 native_handle_value = native_handle.HasValue ? native_handle.Value.ToInt64() : 0;
        Logger.WriteTrace($"Searching for object with handle: {handle}, entity_id: {entity_id}, native_handle: 0x{native_handle_value:x}");
        List<ObjectKey> keys = new List<ObjectKey>();
        foreach (ObjectKey key in keys) {
          if (native_handle != null && key.native_handle == native_handle) {
            return objects[key];
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
          throw new AmbiguousHandleException("Ambiguous Object Handle found!" , keys);
        }

        return objects[keys[0]];
      } catch (AmbiguousHandleException e) {
        Logger.WriteError(e.Message);
        foreach (ObjectKey key in e.Keys) {
          Logger.WriteError($"Handle: {key.handle}, Entity ID: {key.entity_id}, Native Handle: 0x{key.native_handle.ToInt64:x}");
        }
        return null;
      }
    }
#nullable disable

    public static void Unregister(OtherBehavior behavior) {
      unsafe {
        if (behavior.NativeHandle == IntPtr.Zero) {
          Logger.WriteError("Unregistering object with invalid handle");
          return;
        }
        objects.Remove(new ObjectKey(behavior.ObjectID, behavior.EntityID, behavior.NativeHandle));
      }
    }
  }

    public class AmbiguousHandleException : Exception {
    private List<ObjectKey> keys = new List<ObjectKey>();
    public List<ObjectKey> Keys {
      get => keys;
    }

    public AmbiguousHandleException(string message , List<ObjectKey> keys)  
        : base(message) {
      this.keys = keys;
    }
  } 

  public struct ObjectKey : IEquatable<ObjectKey> {
    public readonly UInt64 handle;
    public readonly UInt32 entity_id;
    public readonly IntPtr native_handle;

    public ObjectKey(OtherObject obj) {
      handle = obj.ObjectID;
      entity_id = obj.EntityID;
      native_handle = obj.NativeHandle;
    }

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

}