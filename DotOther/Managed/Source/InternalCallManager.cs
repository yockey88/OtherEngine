using System;
using System.Reflection;
using System.Linq;
using System.Runtime.InteropServices;
using DotOther.Managed.Interop;

namespace DotOther.Managed {

  using static DotOtherHost;
  
  internal static class InternalCallManager {
    private static void RegisterInternalCall(InternalCall call) {
      try {
        var name = call.Name;

        if (name == null) {
          LogMessage($"Cannot register internal call with null name!", MessageLevel.Error);
          return;
        }

        var name_start = name.IndexOf('+');
        var name_end = name.IndexOf(",", name_start, StringComparison.CurrentCulture);
        var field_name = name.Substring(name_start + 1, name_end - name_start - 1);
        var containing_type_name = name.Remove(name_start, name_end - name_start);

        var type = InteropInterface.FindType(containing_type_name);

        if (type == null) {
          LogMessage($"Cannot register internal call '{name}', failed to find type '{containing_type_name}'.", MessageLevel.Error);
          return;
        }

        var binding_flags = BindingFlags.Static | BindingFlags.NonPublic;
        var field = type.GetFields(binding_flags).FirstOrDefault(field => field.Name == field_name);

        if (field == null) {
          LogMessage($"Cannot register internal '{name}', failed to find it in type '{containing_type_name}'", MessageLevel.Error);
          return;
        }

        if (!field.FieldType.IsFunctionPointer) {
          LogMessage($"Field '{name}' is not a function pointer type!", MessageLevel.Error);
          return;
        }

        LogMessage($"Registering internal call '{name}' in type '{containing_type_name}'...", MessageLevel.Info);
        field.SetValue(null, call.Target);
      } catch (Exception ex) {
        HandleException(ex);
      }
    }

    [UnmanagedCallersOnly]
    private static unsafe void SetInternalCalls(IntPtr internall_calls, int length) {
      var calls = new NArray<InternalCall>(internall_calls, length);

      try {
        for (int i = 0; i < calls.Length; i++) {
          RegisterInternalCall(calls[i]);
        }
      } catch (Exception ex) {
        HandleException(ex);
      }
    }

    [UnmanagedCallersOnly]
    private static unsafe void SetInternalCall(InternalCall internal_call) {
      try {
        RegisterInternalCall(internal_call);
      } catch (Exception ex) {
        HandleException(ex);
      }
    }
  }

}