using System;
using System.Runtime.InteropServices;

/// <summary>
/// This namespace contains the managed code for the DotOther library.
/// 
/// Naming Convention:
///   - All typesused for internal interop prefixed with 'N'.
/// </summary>

using DotOther.Managed.Interop;

namespace DotOther.Managed {

  internal enum MessageLevel {
    Info = 1,
    Warning = 2,
    Error = 4
  }

  [StructLayout(LayoutKind.Sequential, Pack = 1)]
  struct DotOtherArgs {
    public unsafe delegate*<NString , void> ExceptionCallback;
    public unsafe delegate*<NString , Int32, void> LogCallback;
  }


  internal static class DotOtherHost {
    private static unsafe delegate*<NString , void> ExceptionCallback;
    private static unsafe delegate*<NString , Int32, void> LogCallback;

    [UnmanagedCallersOnly]
    private static unsafe void EntryPoint(DotOtherArgs args) {
      ExceptionCallback = args.ExceptionCallback;
      LogCallback = args.LogCallback;
      LogCallback("DotOtherHost: Initialized", (Int32)MessageLevel.Info);
    }

    internal static void LogMessage(string message, MessageLevel level) {
      unsafe {
        NString msg = message;
        LogCallback(msg, (Int32)level);
      }
    }

    internal static void HandleException(Exception e) {
      unsafe {
        if (ExceptionCallback == null) {
          return;
        }

        NString msg = e.Message;
        ExceptionCallback(msg);
      }
    }
  }

}