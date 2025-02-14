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

    public static OtherObject GetObject(UInt64 handle, IntPtr native_handle) {
      ObjectKey key = new ObjectKey(handle, native_handle);
      if (objects.ContainsKey(key)) {
        return objects[key];
      }
      return null;
    }

#nullable enable
    public static OtherObject? LookUp(UInt64 handle) {
      foreach (ObjectKey key in objects.Keys) {
        if (key.handle == handle) {
          return objects[key];
        }
      }
      return null;
    }

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
          bool native_handle_match = native_handle == null || key.native_handle == native_handle;
          if (handle_match || native_handle_match) {
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
          Logger.WriteError($"Handle: {key.handle}, Native Handle: 0x{key.native_handle.ToInt64:x}");
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
        objects.Remove(new ObjectKey(behavior.ObjectID, behavior.NativeHandle));
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
    public readonly IntPtr native_handle;
    
    public ObjectKey(OtherObject obj) {
      handle = obj.ObjectID;
      native_handle = obj.NativeHandle;
    }

    public ObjectKey(UInt64 handle, IntPtr native_handle) {
      this.handle = handle;
      this.native_handle = native_handle;
    }

    public bool Equals(ObjectKey other) => native_handle == other.native_handle && handle == other.handle;
    public override bool Equals(object obj) => obj is ObjectKey other && Equals(other);

    public override int GetHashCode() => HashCode.Combine(handle , native_handle);

    public static bool operator==(ObjectKey left, ObjectKey right) => left.Equals(right);
    public static bool operator!=(ObjectKey left, ObjectKey right) => !(left == right);
  }

}