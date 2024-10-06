using System;
using System.Collections.Generic;

namespace Other {
  
  public class Relationship : Component {
    public Relationship() : base(null) {
    }
    
    public Relationship(OtherObject obj) : base(obj) {
    }

    static internal unsafe delegate*<IntPtr , UInt64> GetParent;

    static internal unsafe delegate*<IntPtr , UInt64* , UInt64* , void> GetChildren;

    public UUID Parent {
      get {
        unsafe {
          return GetParent(Object.NativeHandle);
        }
      }
    }

    public List<UUID> Children {
      get {
        unsafe {
          UInt64 count = 0;
          GetChildren(Object.NativeHandle, null, &count);
          UInt64* child_ids = stackalloc UInt64[(int)count];
          GetChildren(Object.NativeHandle, child_ids, &count);
          
          List<UUID> children = new List<UUID>();
          for (UInt64 i = 0; i < count; i++) {
            children.Add(child_ids[i]);
          }
          return children;
        }
      }
    }
  }

}