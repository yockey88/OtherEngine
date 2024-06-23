using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;

namespace Other {

  public class Scene {

    private static Dictionary<ulong , OtherObject> objects = new Dictionary<ulong , OtherObject>();
    
    /// native getters    
    [MethodImpl(MethodImplOptions.InternalCall)]
    public static extern string NativeGetName(ulong id);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void NativeGetScale(ulong id , out Vec3 scale);    

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void NativeGetPosition(ulong id , out Vec3 position);    

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void NativeGetRotation(ulong id , out Vec3 rotation);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern ulong NativeGetObjectByName(string name);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern bool NativeHasObjectById(ulong id);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern bool NativeHasObjectByName(string name);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern ulong NativeGetParent(ulong id);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern ulong NativeGetNumChildren(ulong id);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern ulong[] NativeGetChildren(ulong id);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void NativeAddComponent(ulong id , Type type);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern bool NativeHasComponent(ulong id , Type type);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void NativeRemoveComponent(ulong id , Type type);

    /// C# getters
    static public string GetName(ulong id) {
      return NativeGetName(id);
    }

    static public void GetPosition(ulong id , out Vec3 position) {
      NativeGetPosition(id , out position);
    }

    static public void GetScale(ulong id , out Vec3 scale) {
      NativeGetScale(id , out scale);
    }

    static public void GetRotation(ulong id , out Vec3 rotation) {
      NativeGetRotation(id , out rotation);
    }

    static public OtherObject GetObject(ulong id) {
      if (!HasObject(id)) {
        return null;
      }

      if (objects.ContainsKey(id)) {
        return objects[id];
      } else {
        OtherObject obj = new OtherObject();
        obj.ObjectID = id;
        objects.Add(id , obj);
        return obj;
      }
    }

    static public OtherObject GetObject(string name) {
      if (!HasObject(name)) {
        return null;
      }

      ulong id = NativeGetObjectByName(name);
      if (objects.ContainsKey(id)) {
        return objects[id];
      } else {
        OtherObject obj = new OtherObject();
        obj.ObjectID = id;
        objects.Add(id , obj);
        return obj;
      }
    }

    static public bool HasObject(ulong id) {
      return NativeHasObjectById(id);
    }

    static public bool HasObject(string name) {
      return NativeHasObjectByName(name);
    }

    static public OtherObject GetParent(ulong id) {
      ulong pid = NativeGetParent(id);
      if (objects.ContainsKey(pid)) {
        return objects[pid];
      } else if (pid == 0) {
        return null;
      }

      OtherObject obj = new OtherObject();
      obj.ObjectID = pid;
      objects.Add(pid , obj);
      return null;
    }

    static public ulong GetNumChildren(ulong id) {
      return NativeGetNumChildren(id);
    }

    static public List<OtherBehavior> GetChildren(ulong id) {
      ulong[] ids = NativeGetChildren(id);

      List<OtherBehavior> children = new List<OtherBehavior>();

      foreach (ulong child_id in ids) {
        if (objects.ContainsKey(child_id)) {
          children.Add(objects[child_id]);
        } else {
          OtherObject obj = new OtherObject{ ObjectID = child_id };
          objects.Add(child_id , obj);
          children.Add(obj);
        }
      }

      return children;
    }

    static public void AddComponent(ulong id , Type type) {
      NativeAddComponent(id , type);
    }

    static public bool HasComponent(ulong id , Type type) {
      return NativeHasComponent(id , type);
    }

    static public void RemoveComponent(ulong id , Type type) {
      NativeRemoveComponent(id , type);
    }

    /// native setters

    [MethodImpl(MethodImplOptions.InternalCall)]
    public static extern void NativeSetName(ulong id, string name);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void NativeSetScale(ulong id , ref Vec3 scale);    
    

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void NativeSetPosition(ulong id , ref Vec3 position);    

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void NativeSetRotation(ulong id , ref Vec3 rotation);   

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void NativeSetParent(ulong id , ref Vec3 scale);

    /// C# setters 

    static public void SetPosition(ulong id , ref Vec3 position) {
      NativeSetPosition(id , ref position);
    }

    static public void SetScale(ulong id , ref Vec3 scale) {
      NativeSetScale(id , ref scale);
    }

    static public void SetRotation(ulong id , ref Vec3 rotation) {
      NativeSetRotation(id , ref rotation);
    }

    static public void InitializeScene() {
      try {
        objects.Clear();
      } catch (Exception e) {
        Logger.WriteError($"Failed to initialize scene: {e.Message}");
      }
    }

    /// other
    static public void AddObject(OtherObject obj) {
      try {
        objects.Add(obj.ObjectID , obj);
      } catch (Exception e) {
        Logger.WriteError($"Failed to add object to scene: {e.Message}");
      }
    }

  }

}