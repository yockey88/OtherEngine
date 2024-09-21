using System;
using System.Runtime.InteropServices;

using DotOther.Managed.Interop;

namespace DotOther {

  public class NObject {
    private UInt64 native_handle;

    public UInt64 NHandle {
      get => native_handle;
    }

    public NObject(UInt64 ptr) {
      native_handle = ptr;
    }
  }

  internal static class NObjectInterface {
    private static unsafe delegate*<NObject> CreateObject;
  }

}