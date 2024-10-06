using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

using DotOther.Managed;

namespace Other {

  [StructLayout(LayoutKind.Sequential , Size = 8)]
  public class EntityHandle {
    public readonly UInt64 internal_id;
    public readonly UInt32 entity_id;
    internal readonly IntPtr handle;

    OtherObject object_ref = null;

    private Dictionary<Type , Component> components;

    public EntityHandle(OtherObject object_ref) {
      internal_id = object_ref.ObjectID;
      entity_id = object_ref.EntityID;
      handle = object_ref.NativeHandle;
      this.object_ref = object_ref;
    }

    public EntityHandle(IntPtr handle) {
      object_ref = ObjectRegistry.TryGetObject(null , null , handle);
      if (object_ref == null) {
        Logger.WriteError($"Invalid handle: 0x{handle.ToInt64():x}");
        handle = IntPtr.Zero;
        internal_id = 0;
        entity_id = 0;
        return;
      }

      handle = object_ref.NativeHandle;
      internal_id = object_ref.ObjectID;
      entity_id = object_ref.EntityID;
    }

    public EntityHandle(UInt64 internal_id , UInt32 entity_id , IntPtr handle) {
      this.internal_id = internal_id;
      this.entity_id = entity_id;
      this.handle = handle;
      object_ref = ObjectRegistry.TryGetObject(internal_id , entity_id , handle);

      if (object_ref == null) {
        this.internal_id = 0;
        this.entity_id = 0;
        Logger.WriteError($"Invalid handle: 0x{handle.ToInt64():x}");
        return;
      }
    }

    public static implicit operator OtherObject(EntityHandle handle) => handle.object_ref;
  }

}