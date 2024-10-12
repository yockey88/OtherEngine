using System;
using System.Runtime.InteropServices;
using System.Text;

namespace DotOther.Managed {

  internal enum MessageLevel {
    Trace = 0,
    Debug = 1,
    Info = 2,
    Warning = 3,
    Error = 4,
    Critical = 5,

    Message = 6,
    Bad = 7,
    All = 8
  }

  [StructLayout(LayoutKind.Sequential, Pack = 1)]
  struct DotOtherArgs {
    public unsafe delegate*<NString , void> ExceptionCallback;
    public unsafe delegate*<NString , MessageLevel, void> LogCallback;
    public unsafe delegate*<UInt64, NString, void> NativeMethodInvoker;
    public unsafe delegate*<NObject,UInt64> RetrieveNativeObject;
  }


  internal static class DotOtherHost {
    private static unsafe delegate*<NString , void> ExceptionCallback;
    private static unsafe delegate*<NString , MessageLevel, void> LogCallback;

    private static unsafe delegate*<UInt64, NString, void> NativeMethodInvoker;
    private static unsafe delegate*<NObject,UInt64> RetrieveNativeObject;

    [UnmanagedCallersOnly]
    private static unsafe void EntryPoint(DotOtherArgs args) {
      ExceptionCallback = args.ExceptionCallback;
      LogCallback = args.LogCallback;
      NativeMethodInvoker = args.NativeMethodInvoker;
      RetrieveNativeObject = args.RetrieveNativeObject;
      LogCallback("DotOtherHost: Initialized", MessageLevel.Info);
    }

    internal static void LogMessage(string message, MessageLevel level) {
      unsafe {
        NString msg = message;
        LogCallback(msg, level);
      }
    }

    internal static void HandleException(Exception e) {
      unsafe {
        if (ExceptionCallback == null) {
          return;
        }

        StringBuilder sb = new();
        sb.Append($"{e.Message} =>\n{e.StackTrace}");

        NString msg = sb.ToString();
        ExceptionCallback(msg);
      }
    }

    internal static void InvokeNativeMethod(UInt64 handle, string method) {
      NString method_name = method;

      unsafe {
        if (NativeMethodInvoker == null) {
          LogMessage("NativeMethodInvoker is null", MessageLevel.Error);
          throw new InvalidOperationException("NativeMethodInvoker is null");
        }
        NativeMethodInvoker(handle, method_name);
      }
    }

    internal static NObject GetNativeObject(UInt64 handle) {
      if (handle == 0) {
        return null;
      }

      unsafe {
        if (GetNativeObject == null) {
          LogMessage("GetNativeObject is null", MessageLevel.Error);
          throw new InvalidOperationException("GetNativeObject is null");
        }

        try {
          return GetNativeObject(handle);
        } catch (Exception e) {
          HandleException(e);
          return null;
        }
      }
    }
  }

}