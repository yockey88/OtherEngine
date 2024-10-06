using System.Runtime.CompilerServices;

using DotOther.Managed;

namespace Other {

  internal enum LogLevel {
    Trace = 0 ,
    Debug,
    Info,
    Warning,
    Error,
    Fatal
  }

  public static class Logger {
#region LoggingUtilities

    internal static unsafe delegate*<NString , LogLevel , void> Write;

    public static void WriteTrace(string line) {
      unsafe {
        NString msg = line;
        Write(msg, LogLevel.Trace);
      }
    }

    public static void WriteDebug(string line) {
      unsafe {
        NString msg = line;
        Write(msg, LogLevel.Debug);
      }
    }

    public static void WriteInfo(string line) {
      unsafe {
        NString msg = line;
        Write(msg, LogLevel.Info);
      }
    }

    public static void WriteWarning(string line) {
      unsafe {
        NString msg = line;
        Write(msg, LogLevel.Warning);
      }
    }

    public static void WriteError(string line) {
      unsafe {
        NString msg = line;
        Write(msg, LogLevel.Error);
      }
    }

    public static void WriteFatal(string line) {
      unsafe {
        NString msg = line;
        Write(msg, LogLevel.Fatal);
      }
    }

#endregion /// LoggingUtilities
  }

}
