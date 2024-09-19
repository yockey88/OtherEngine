using System;
using System.Collections.Generic;
using System.IO;
using System.IO.MemoryMappedFiles;
using System.Linq;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Runtime.Loader;

using DotOther.Managed.Interop;

namespace DotOther.Managed {

  using static DotOtherHost;

  public enum AsmLoadStatus {
    Success ,
    NotFound , 
    Failed ,
    InvalidPath ,
    InvalidAssembly ,
    UnknownError
  }

  public static class AssemblyLoader {
    private static readonly Dictionary<Type, AsmLoadStatus> load_errors = new();
    private static readonly Dictionary<Int32, AssemblyLoadContext> contexts = new();
    private static readonly Dictionary<Int32, Assembly> assemblies = new();
    private static Dictionary<Int32, List<GCHandle>> handles = new();

    private static AsmLoadStatus last_load_status = AsmLoadStatus.Success;
#nullable enable
    private static readonly AssemblyLoadContext? dotother_asm_context;
#nullable disable

    static AssemblyLoader() {
    }

    private static void CacheDotOtherAssemblies() {
    }


#nullable enable
    internal static bool TryGetAssembly(Int32 id , out Assembly? asm) {
      return assemblies.TryGetValue(id, out asm);
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
    private static Int32 CreateAssemblyLoadContext(NString context_name) {
      string? name = context_name;

      if (name == null) {
        return -1;
      }

      var alc = new AssemblyLoadContext(name, true);

      alc.Resolving += ResolveAssembly;
      alc.Unloading += ctx => {
        foreach (var asm in ctx.Assemblies) {
          var asm_name = asm.GetName();
          Int32 asm_id = asm_name.Name!.GetHashCode();
          assemblies.Remove(asm_id);
        }
      };

      Int32 ctx_id = name.GetHashCode();
      contexts.Add(ctx_id, alc);

      return ctx_id;
    }

    
	  [UnmanagedCallersOnly]
	  private static void UnloadAssemblyLoadContext(Int32 context_id) {
      if (!contexts.TryGetValue(context_id, out var alc)) {
        LogMessage($"Cannot unload AssemblyLoadContext '{context_id}', it was either never loaded or already unloaded.", MessageLevel.Warning);
        return;
      }

      if (alc == null) {
        LogMessage($"AssemblyLoadContext '{context_id}' was found in dictionary but was null. This is most likely a bug.", MessageLevel.Error);
        return;
      }

      foreach (var assembly in alc.Assemblies) {
        var asm_name = assembly.GetName();
        int asm_id = asm_name.Name!.GetHashCode();

        if (!handles.TryGetValue(asm_id, out var hs)) {
          continue;
        }

        foreach (var h in hs) {
          if (!h.IsAllocated || h.Target == null) {
            continue;
          }

          LogMessage($"Found unfreed object '{h.Target}' from assembly '{asm_name}'. Deallocating.", MessageLevel.Warning);
          h.Free();
        }
      }

      // ManagedObject.s_CachedMethods.Clear();

      InteropInterface.cached_types.Clear();
      InteropInterface.cached_methods.Clear();
      InteropInterface.cached_fields.Clear();
      InteropInterface.cached_properties.Clear();
      InteropInterface.cached_attributes.Clear();

      contexts.Remove(context_id);
      alc.Unload();
    }
    
    [UnmanagedCallersOnly]
    private static Int32 LoadAssembly(Int32 context_id, NString file_path) {
      try {
        if (string.IsNullOrEmpty(file_path)) {
          last_load_status = AsmLoadStatus.InvalidPath;
          return -1;
        }

        if (!File.Exists(file_path)) {
          LogMessage($"Failed to load assembly : '{file_path}', file not found", MessageLevel.Error);
          return -1;
        }

        if (!contexts.TryGetValue(context_id, out var alc)) {
          LogMessage($"Failed to load assembly '{file_path}', couldn't find Load Context with id '{context_id}'", MessageLevel.Error);
          return -1;
        }

        if (alc == null) {
          LogMessage($"Failed to load assembly '{file_path}', Load Context with id '{context_id}' is null", MessageLevel.Error);
          return -1;
        }

        Assembly? asm = null;

        using (var file = MemoryMappedFile.CreateFromFile(file_path!)) {
          using var stream = file.CreateViewStream();
          asm = alc.LoadFromStream(stream);
        }

        var name = asm.GetName();
        LogMessage($"Loading assembly : '{name}' [{context_id}]", MessageLevel.Info);
        Int32 asm_id = name.Name!.GetHashCode();
        assemblies.Add(asm_id, asm);
        last_load_status = AsmLoadStatus.Success;
        return asm_id;
      } catch (Exception e) {
        HandleException(e);
        return -1;
      }
    }

    [UnmanagedCallersOnly]
    private static AsmLoadStatus GetLastLoadStatus() => last_load_status;

    [UnmanagedCallersOnly]
    private static NString GetAsmName(Int32 asm_id) {
      if (!assemblies.TryGetValue(asm_id, out var asm)) {
        LogMessage($"Couldn't get assembly name for assembly '{asm_id}', assembly not found!", MessageLevel.Error);
        return "<unknown>";
      }

      var asm_name = asm.GetName();
      return asm_name.Name;
    }

    internal static void RegisterHandle(Assembly asm , GCHandle handle) {
      var asm_name = asm.GetName();
      Int32 asm_id = asm_name.Name!.GetHashCode();

      if (!handles.TryGetValue(asm_id , out var hs)) {
        handles.Add(asm_id, new List<GCHandle>());
        hs = handles[asm_id];
      }

      hs.Add(handle);
    }
#nullable disable
  }

}