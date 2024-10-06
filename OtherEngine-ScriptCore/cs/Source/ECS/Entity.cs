using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

using DotOther.Managed;

namespace Other {

  [StructLayout(LayoutKind.Sequential , Size = 8)]
  public class EntityHandle {

    OtherObject object_ref = null;

    private Dictionary<Type , Component> components;

    public EntityHandle(OtherObject object_ref) {
      this.object_ref = object_ref;
    }

    public EntityHandle(IntPtr handle) {
      object_ref = ObjectRegistry.TryGetObject(null , null , handle);
      
      if (object_ref == null) {
        Logger.WriteError($"Invalid handle: 0x{handle.ToInt64():x}");
      }
    }

    public EntityHandle(UInt64 internal_id , UInt32 entity_id , IntPtr handle) {
      object_ref = ObjectRegistry.TryGetObject(internal_id , entity_id , handle);

      if (object_ref == null) {
        Logger.WriteError($"Invalid handle: 0x{handle.ToInt64():x}");
      }
    }

    public static implicit operator OtherObject(EntityHandle handle) => handle.object_ref;
  }

}