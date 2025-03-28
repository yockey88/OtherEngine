using System;
using System.Collections.Generic;
using System.Diagnostics.CodeAnalysis;
using System.Text;
using DotOther.Managed;

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
    internal static unsafe delegate*<UInt64, UInt32> GetSceneId;
    internal static unsafe delegate*<IntPtr , NBool32> IsHandleValid;

    internal static unsafe delegate*<bool> IsSceneActiveAndValid;

    private static SortedSet<UInt64> active_scripts = new SortedSet<UInt64>();
    private static Dictionary<UInt64 , OtherObject> objects = new Dictionary<UInt64 , OtherObject>();

    public override List<OtherBehavior> Children { 
      get => throw new NotImplementedException(); 
      set => throw new NotImplementedException(); 
    }

    public override OtherBehavior Parent { 
      get => throw new NotImplementedException(); 
      set => throw new NotImplementedException(); 
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

    // public static EntityHandle GetEntityHandle(UUID id) {
    //   unsafe {
    //     if (objects.TryGetValue(id , out OtherObject obj)) {
    //       Logger.WriteDebug($"Creating Entity Handle for Object : {obj.Name} [{obj.ObjectID}]");
    //       return new EntityHandle(obj);
    //     }

    //     obj = ObjectRegistry.TryGetObject(id , null , null);
    //     if (obj != null) {
    //       Logger.WriteDebug($"Creating Entity Handle for Object : {obj.Name} [{obj.ObjectID}]");
    //       return new EntityHandle(obj);
    //     }

    //     IntPtr native_handle = GetNativeHandle(id);
    //     if (native_handle != IntPtr.Zero) {
    //       OtherObject other_object = new OtherObject(native_handle);
    //       Logger.WriteDebug($"Creating Entity Handle for Object : {other_object.Name} [{other_object.ObjectID}]");
    //       return new EntityHandle(other_object);
    //     }

    //     Logger.WriteError($"Failed to get entity handle for {id}");
    //     return null;
    //   }
    // }


    public static void AddActiveScript(UInt64 id) {
      if (objects.ContainsKey(id)) {
        active_scripts.Add(id);
      } else {
        Logger.WriteError($"Failed to add active script : {id} (object not found)");
      }
    }

    public static void RemoveActiveScript(UInt64 id) {
      if (active_scripts.Contains(id)) {
        Logger.WriteTrace($"Removing Active Script : {id}");
        active_scripts.Add(id);
      }
    }

    public static bool IsActive() {
      unsafe {
        return IsSceneActiveAndValid();
      }
    }

#nullable enable
    public static OtherObject? GetObject(UInt64 id) {
      if (objects.TryGetValue(id , out OtherObject? obj)) {
        Logger.WriteDebug($"Found Object : {obj.Name} [{id}]");
        return obj;
      }
      Logger.WriteError($"Failed to get object : {id}");
      return null;
    }
#nullable disable

    public void RegisterSceneObject(UInt64 id) {
      if (objects.ContainsKey(id)) {
        return;
      }

      unsafe {
        IntPtr native_handle = GetNativeHandle(id);
        if (native_handle == IntPtr.Zero) {
          Logger.WriteError($"Failed to register scene object : {id}");
          return;
        }
        UInt32 entity_id = GetSceneId(id);

        OtherObject obj = new OtherObject(native_handle, id, entity_id);
        Logger.WriteTrace($"Registering Scene Object : {obj.Name} [{id}] [Scene Id : {entity_id}] [Native Handle : 0x{native_handle.ToInt64():x}]");
        objects.Add(id , obj);

        if (obj.HasComponent<Script>()) {
          Logger.WriteTrace($"Adding Active Script for [{obj.Name}] : {id}");
          AddActiveScript(id);
        }
      }
    }

    public void ClearObjects() {
      objects.Clear();
    }

    public void ListObjects() {
      foreach (KeyValuePair<UInt64 , OtherObject> obj in objects) {
        StringBuilder sb = new StringBuilder();
        sb.Append($"Object : {obj.Value.Name} [{obj.Key}]");
        if (active_scripts.Contains(obj.Key)) {
          sb.Append(" (Active Script)");
        }

        Logger.WriteTrace(sb.ToString());
      }
    }

    public override void OnInitialize() {
      Console.WriteLine("Scene Initialized");
    }

    public override void OnShutdown() {
      Console.WriteLine("Scene Shutdown");
    }
  }

}