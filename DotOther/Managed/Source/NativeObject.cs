using System;
using System.Runtime.InteropServices;

using DotOther.Managed.Interop;

namespace DotOther {

  [StructLayout(LayoutKind.Sequential, Pack = 1)]
  public class NObject {
    private UInt64 native_handle;

    public UInt64 NHandle {
      get => native_handle;
    }

    public NObject() {
    }

    public void Invoke(string method) {
      try {
      } catch (Exception e) {
        Console.WriteLine($"Exception in Invoke: {e.Message} |\n{e.StackTrace}");
      }
    }
  }

}