using System.Runtime.CompilerServices;

namespace Other {

  public static class Logger {
#region LoggingUtilities

    [MethodImpl(MethodImplOptions.InternalCall)]
    public static extern void WriteLine(string line);

    [MethodImpl(MethodImplOptions.InternalCall)]
    public static extern void WriteTrace(string line);

    [MethodImpl(MethodImplOptions.InternalCall)]
    public static extern void WriteDebug(string line);

    [MethodImpl(MethodImplOptions.InternalCall)]
    public static extern void WriteInfo(string line);

    [MethodImpl(MethodImplOptions.InternalCall)]
    public static extern void WriteWarn(string line);

    [MethodImpl(MethodImplOptions.InternalCall)]
    public static extern void WriteError(string line);

    [MethodImpl(MethodImplOptions.InternalCall)]
    public static extern void WriteFatal(string line);

#endregion /// LoggingUtilities
  }

}
