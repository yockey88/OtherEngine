using System;
using System.Runtime.InteropServices;

namespace DotOther.Managed {

  internal static class GarbageCollector {
    [UnmanagedCallersOnly]
    internal static void CollectGarbage(Int32 generation, GCCollectionMode mode, NBool32 blocking, NBool32 compacting) {
      try { 
        if (generation < 0) {
          GC.Collect();
        } else {
          GC.Collect(generation, mode, blocking, compacting);
        }
      } catch (Exception e) {
        DotOtherHost.HandleException(e);
      }
    }

    [UnmanagedCallersOnly]
    internal static void WaitForPendingFinalizers() {
      try {
        GC.WaitForPendingFinalizers();
      } catch (Exception e) {
        DotOtherHost.HandleException(e);
      }
    }
  }

}