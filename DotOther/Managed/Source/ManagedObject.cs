using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;
using System.Reflection;
using System.Reflection.Metadata.Ecma335;
using System.Runtime.InteropServices;

using DotOther.Managed.Interop;

namespace DotOther.Managed {

using static DotOtherHost;

internal enum ManagedType {
	Unknown,

	SByte,
	Byte,
	Short,
	UShort,
	Int,
	UInt,
	Long,
	ULong,

	Float,
	Double,

	Bool,

	Pointer
};

internal static class ManagedObject {

	public readonly struct MethodKey : IEquatable<MethodKey> {
		public readonly string type_name;
		public readonly string name;
		public readonly ManagedType[] arg_types;
		public readonly Int32 param_count;

		public MethodKey(string type_name, string name, ManagedType[] arg_types, Int32 param_count) {
			this.type_name = type_name;
			this.name = name;
			this.arg_types = arg_types;
			this.param_count = arg_types.Length;
		}

		public override bool Equals([NotNullWhen(true)] object? obj) => obj is MethodKey key && Equals(key);

		bool IEquatable<MethodKey>.Equals(MethodKey other) {
			if (type_name != other.type_name || name != other.name || param_count != other.param_count) {
				return false;
			}

			for (Int32 i = 0; i < param_count; i++) {
				if (arg_types[i] != other.arg_types[i]) {
					return false;
				}
			}

			return true;
		}

		public override int GetHashCode() {
			return base.GetHashCode();
		}

		internal static Dictionary<MethodKey , MethodInfo> methods = new Dictionary<MethodKey, MethodInfo>();

#nullable enable
		private static unsafe MethodInfo? TruGetMethodInfo(Type type, string name, ManagedType* types, Int32 count, BindingFlags flags) {
			MethodInfo? minfo = null;

			var param_types = new ManagedType[count];

			unsafe {
				fixed (ManagedType* param_types_ptr = param_types) {
					ulong size = sizeof(ManagedType) * (ulong)count;
					Buffer.MemoryCopy(types, param_types_ptr, size, size);
				}
			}

			var mkey = new MethodKey(type.FullName, name, param_types, count);

			if (!methods.TryGetValue(mkey, out minfo)) {
				List<MethodInfo> methods = new(type.GetMethods(flags));

				Type? baseType = type.BaseType;
				while (baseType != null) {
					methods.AddRange(baseType.GetMethods(flags));
					baseType = baseType.BaseType;
				}

				minfo = InteropInterface.FindSuitableMethod<MethodInfo>(name, types, count, CollectionsMarshal.AsSpan(methods));

				if (minfo == null) {
					LogMessage($"Failed to find method '{name}' for type {type.FullName} with {count} parameters.", MessageLevel.Error);
					return null;
				}

				// methods.Add(mkey, minfo);
			}

			return minfo;
		}

		[UnmanagedCallersOnly]
		private static unsafe IntPtr CreateObject(Int32 typeid, NBool32 weak_ref, IntPtr parameters, ManagedType* param_types, Int32 count) {
			try {
				if (!InteropInterface.cached_types.TryGet(typeid, out var type)) {
					LogMessage($"Type with ID '{typeid}' not found in cache.", MessageLevel.Error);
					return IntPtr.Zero;
				}

				ConstructorInfo? ctor = null;
				var curr_type = type;
				while (curr_type != null) {
					ReadOnlySpan<ConstructorInfo> ctors = curr_type.GetConstructors(BindingFlags.NonPublic | BindingFlags.Public | BindingFlags.Instance);
					ctor = InteropInterface.FindSuitableMethod(".ctor", param_types, count, ctors);
					if (ctor != null) {
						break;
					}

					curr_type = curr_type.BaseType;
				}

				if (ctor == null) {
					LogMessage($"No suitable constructor found for type '{type.FullName}'.", MessageLevel.Error);
					return IntPtr.Zero;
				}

				var marshalled_parameters = Interop.DotOtherMarshal.MarshalParameterArray(parameters, count, ctor);
				object? result = null;

				if (curr_type != type || marshalled_parameters == null) {
					result = InteropInterface.CreateInstance(type);
					if (curr_type != type) {
						ctor.Invoke(result, marshalled_parameters);
					}
				} else {
					result = InteropInterface.CreateInstance(type, marshalled_parameters);
				}

				if (result == null) {
					LogMessage($"Failed to create instance of type '{type.FullName}'.", MessageLevel.Error);
					return IntPtr.Zero;
				}

				var handle = GCHandle.Alloc(result, weak_ref ? GCHandleType.Weak : GCHandleType.Normal);
				AssemblyLoader.RegisterHandle(type.Assembly, handle);
				return GCHandle.ToIntPtr(handle);
			} catch (Exception e) {
				DotOtherHost.HandleException(e);
				return IntPtr.Zero;
			}
		}

		[UnmanagedCallersOnly]
		private static unsafe void DestroyObject(IntPtr handle) {
			try {
				GCHandle.FromIntPtr(handle).Free();
			} catch (Exception e) {
				DotOtherHost.HandleException(e);
			}
		}

		[UnmanagedCallersOnly]
		private static unsafe void InvokeMethod(IntPtr handle, NString method_name, IntPtr parameters, ManagedType* param_types, int count) {
			try {
				var target = GCHandle.FromIntPtr(handle).Target;

				if (target == null) {
					LogMessage($"Cannot invoke method {method_name} on a null type.", MessageLevel.Error);
					return;
				}

				var targetType = target.GetType();

				// var minfo = TryGetMethodInfo(targetType, method_name, param_types, count, BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance);
				// var marshaleld_parameters = DotOtherMarshal.MarshalParameterArray(parameters, count, minfo);

				// minfo.Invoke(target, parameters);
			} catch (Exception ex) {
				HandleException(ex);
			}
		}
#nullable disable
	}

}

}