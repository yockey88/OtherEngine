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

    private static string FormatLogMessage(string line, string src_loc) {
      return $"{line} [{src_loc}]";
    }

    public static void WriteTrace(string line , [CallerFilePath] string loc = null) {
      unsafe {
        NString msg = FormatLogMessage(line, loc);
        Write(msg, LogLevel.Trace);
      }
    }

    public static void WriteDebug(string line , [CallerFilePath] string loc = null) {
      unsafe {
        NString msg = FormatLogMessage(line, loc);
        Write(msg, LogLevel.Debug);
      }
    }

    public static void WriteInfo(string line , [CallerFilePath] string loc = null) {
      unsafe {
        NString msg = FormatLogMessage(line, loc);
        Write(msg, LogLevel.Info);
      }
    }

    public static void WriteWarning(string line , [CallerFilePath] string loc = null) {
      unsafe {
        NString msg = FormatLogMessage(line, loc);
        Write(msg, LogLevel.Warning);
      }
    }

    public static void WriteError(string line , [CallerFilePath] string loc = null) {
      unsafe {
        NString msg = FormatLogMessage(line, loc);
        Write(msg, LogLevel.Error);
      }
    }

    public static void WriteFatal(string line , [CallerFilePath] string loc = null) {
      unsafe {
        NString msg = FormatLogMessage(line, loc);
        Write(msg, LogLevel.Fatal);
      }
    }

#endregion /// LoggingUtilities
  }

}
