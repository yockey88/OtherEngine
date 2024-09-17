using System;
using System.Collections.Generic;
using System.IO;
using System.IO.MemoryMappedFiles;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Runtime.Loader;

using DotOther.Managed.Interop;

namespace DotOther.Managed {


  public enum AsmLoadStatus {
    Success ,
    NotFound , 
    Failed ,
    InvalidPath ,
    InvalidAssembly ,
    UnknownError
  }

  public static class AssemblyLoader {
    private static readonly Dictionary<Type, AsmLoadStatus> AssemblyLoadErrors = new();
    private static readonly Dictionary<string, AssemblyLoadContext> AssemblyContexts = new();
    private static readonly Dictionary<string, Assembly> LoadedAssemblies = new();
    private static Dictionary<Int32, List<GCHandle>> AllocatedHandles = new();

    private static AsmLoadStatus LastDotOtherAsmLoadStatus = AsmLoadStatus.Success;
    private static readonly AsmLoadStatus? DotOtherAsmLoadStatus;

    static AssemblyLoader() {
    }

    private static void CacheDotOtherAssemblies() {
    }

    internal static Assembly? ResolveAssembly(AssemblyLoadContext? context, AssemblyName assemblyName) {
      // if (LoadedAssemblies.TryGetValue(assemblyName.Name, out var asm)) {
      //   return asm;
      // }

      // if (AssemblyContexts.TryGetValue(assemblyName.Name, out var asmCtx)) {
      //   return asmCtx.LoadAssembly(assemblyName);
      // }

      return null;
    }

    [UnmanagedCallersOnly]
    private static Int32 CreateAssemblyLoadContext(NString name) {
      return 1;
    }

    
	  [UnmanagedCallersOnly]
	  private static void UnloadAssemblyLoadContext(Int32 context_id) {
    }
    
    [UnmanagedCallersOnly]
    private static Int32 LoadAssembly(Int32 context_id, NString file_path) {
      return 1;
    }

    [UnmanagedCallersOnly]
    private static AsmLoadStatus GetLastLoadStatus() => LastDotOtherAsmLoadStatus;

    [UnmanagedCallersOnly]
    private static NString GetAsmName(Int32 asm_id) {
      return "DotOther";
    }

    internal static void RegisterHandle(Assembly asm , GCHandle handle) {
      // if (!AllocatedHandles.ContainsKey(asm.GetHashCode())) {
      //   AllocatedHandles.Add(asm.GetHashCode(), new List<GCHandle>());
      // }

      // AllocatedHandles[asm.GetHashCode()].Add(handle);
    }
  }

}