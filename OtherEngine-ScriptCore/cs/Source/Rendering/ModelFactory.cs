using System;
using System.Runtime.CompilerServices;

namespace Other {
  
  public class ModelFactory {
    public ModelFactory() {
    }

    [MethodImpl(MethodImplOptions.InternalCall)]
    public static extern UInt64 NativeCreateBox(ref Vec3 scale);

    static public UUID CreateBox(ref Vec3 scale) {
      try {
        return new UUID(NativeCreateBox(ref scale));
      } catch (Exception e) {
        Logger.WriteError($"Failed to create box : {e}");
        return 0;
      }
    }
  }

}