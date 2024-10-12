using System;
using System.Collections.Generic;
using System.Diagnostics.CodeAnalysis;
using System.Reflection.Metadata;
using DotOther.Managed;
using DotOther.Managed.Interop;

namespace Other {

  public class Scene : OtherBehavior {
    public class ObjectKey : IEquatable<ObjectKey> {
      string name;
      UInt64 id;

      public ObjectKey(string name , UInt64 id) {
        this.name = name;
        this.id = id;
      }

      public override bool Equals([NotNullWhen(true)] object? obj) => obj is ObjectKey && Equals(obj);
      public override int GetHashCode() => HashCode.Combine(name , id);
      bool IEquatable<ObjectKey>.Equals(ObjectKey obj) => obj != null &&  obj.id == id;
    }

    internal static unsafe delegate*<UInt64 , IntPtr> GetNativeHandle;
    internal static unsafe delegate*<IntPtr , NBool32> IsHandleValid;

    private static Dictionary<UInt64 , OtherObject> objects = new Dictionary<UInt64 , OtherObject>();
    
    public override OtherBehavior Parent {
      get => null;
      set {}
    }

    public override List<OtherBehavior> Children {
      get => new List<OtherBehavior>();
      set {}
    }

    public static bool IsValidHandle(UInt64 id) {
      bool exists = objects.ContainsKey(id);
      if (exists) {
        return true;
      }

      unsafe {
        exists = GetNativeHandle(id) != IntPtr.Zero;
      }
      return exists;
    }

    public static bool IsValidHandle(IntPtr handle) {
      unsafe {
        return IsHandleValid(handle);
      }
    }

    public static EntityHandle GetEntityHandle(UUID id) {
      unsafe {
        if (objects.TryGetValue(id , out OtherObject obj)) {
          Logger.WriteDebug($"Creating Entity Handle for Object : {obj.Name} [{obj.ObjectID}]");
          return new EntityHandle(obj);
        }

        obj = ObjectRegistry.TryGetObject(id , null , null);
        if (obj != null) {
          Logger.WriteDebug($"Creating Entity Handle for Object : {obj.Name} [{obj.ObjectID}]");
          return new EntityHandle(obj);
        }

        IntPtr native_handle = GetNativeHandle(id);
        if (native_handle != IntPtr.Zero) {
          OtherObject other_object = new OtherObject(native_handle);
          Logger.WriteDebug($"Creating Entity Handle for Object : {other_object.Name} [{other_object.ObjectID}]");
          return new EntityHandle(other_object);
        }

        Logger.WriteError($"Failed to get entity handle for {id}");
        return null;
      }
    }

    public override void OnShutdown() {
      objects.Clear();
    }

    public static void AddObject(UInt64 id , OtherObject obj) {
      if (!objects.ContainsKey(id)) {
        objects.Add(id , obj);
      }
    }

    public static void RemoveObject(UInt64 id) {
      if (objects.ContainsKey(id)) {
        objects.Remove(id);
      }
    }
  }

}