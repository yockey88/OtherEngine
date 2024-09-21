using System;
using System.Reflection;
using System.Linq;
using System.Runtime.InteropServices;
using DotOther.Managed.Interop;

namespace DotOther.Managed.Interop {

  using static DotOtherHost;
  
#nullable enable
  [StructLayout(LayoutKind.Sequential, Pack = 1)]
	public readonly struct InternalCall {
		private readonly IntPtr NName;
		public readonly IntPtr Target;
		public string? Name => Marshal.PtrToStringAuto(NName);
	}
#nullable disable

  internal static class InternalCallManager {
    private static void RegisterInternalCall(InternalCall call) {
      try {
        var name = call.Name;

        if (name == null) {
          LogMessage($"Cannot register internal call with null name!", MessageLevel.Error);
          return;
        } else {
          LogMessage($"  > Registering internal call '{name}'...", MessageLevel.Info);
        }

        var name_start = name.IndexOf('+');
        var name_end = name.IndexOf(",", name_start, StringComparison.CurrentCulture);
        var field_name = name.Substring(name_start + 1, name_end - name_start - 1);
        var containing_type_name = name.Remove(name_start, name_end - name_start);
        LogMessage($"  > Internal Call : Field Name = {field_name}, Type Name = {containing_type_name}", MessageLevel.Info);

        var type = InteropInterface.FindType(containing_type_name);

        if (type == null) {
          LogMessage($"Cannot register internal call '{name}', failed to find type '{containing_type_name}'.", MessageLevel.Error);
          return;
        } else {
          LogMessage($"  > Found Type : {type}", MessageLevel.Info);
        }

        var binding_flags = BindingFlags.Static | BindingFlags.NonPublic;
        var field = type.GetFields(binding_flags).FirstOrDefault(field => field.Name == field_name);
        if (field == null) {
          LogMessage($"Cannot register internal '{name}', failed to find corresponding delegate in type '{containing_type_name}'", MessageLevel.Error);
          return;
        } else {
          LogMessage($"  > Found Field : {field.Name}", MessageLevel.Info);
        }

        var field_type = field.FieldType;
        if (!field.FieldType.IsFunctionPointer) {
          LogMessage($"Field '{name}' is not a valid delegate for function!", MessageLevel.Error);
          return;
        } else {
          LogMessage($"  > Field '{name}' is a valid delegate", MessageLevel.Info);
        }

        LogMessage($"Internal call '{name}' registered in type '{containing_type_name}'!", MessageLevel.Info);
        field.SetValue(null, call.Target);
      } catch (Exception ex) {
        Console.WriteLine($"Exception: {ex} in {ex.StackTrace}");
        HandleException(ex);
      }
    }

    [UnmanagedCallersOnly]
    private static unsafe void SetInternalCalls(IntPtr internall_calls, int length) {
      Console.WriteLine($"SetInternalCalls: {internall_calls}, {length}");
      var calls = new NArray<InternalCall>(internall_calls, length);

      try {
        for (int i = 0; i < calls.Length; i++) {
          RegisterInternalCall(calls[i]);
        }
      } catch (Exception ex) {
        Console.WriteLine($"Exception: {ex} in {ex.StackTrace}");
        HandleException(ex);
      }
    }

    [UnmanagedCallersOnly]
    private static unsafe void SetInternalCall(InternalCall internal_call) {
      try {
        RegisterInternalCall(internal_call);
      } catch (Exception ex) {
        Console.WriteLine($"Exception: {ex} in {ex.StackTrace}");
        HandleException(ex);
      }
    }
  }

}