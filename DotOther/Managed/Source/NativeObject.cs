using System;
using System.Runtime.InteropServices;
using DotOther.Managed;
using DotOther.Managed.Interop;
using Microsoft.Win32.SafeHandles;

namespace DotOther {

  [StructLayout(LayoutKind.Sequential, Size = 24)]
  public class NObject {
    private readonly IntPtr vtable; //// DO NOT USE
    private UInt64 native_handle;
    private readonly IntPtr proxy_object; //// DO NOT USE

    public UInt64 NHandle {
      get => native_handle;
    }

    public NObject() {
      native_handle = 0;
    }

    public NObject(UInt64 handle) {
      native_handle = handle;
    }

    public void Invoke(string method) {
      try {
        DotOtherHost.InvokeNativeMethod(native_handle, method);
      } catch (Exception e) {
        Console.WriteLine($"Exception in Native Function Invocation: {e.Message} |\n{e.StackTrace}");
      }
    }
  }

}