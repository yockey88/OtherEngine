using System;
using System.Collections.Generic;
using System.Collections.Immutable;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace DotOther.Managed {

using static DotOtherHost;
	[StructLayout(LayoutKind.Sequential, Pack = 1)]
	public readonly struct InternalCall {
		public readonly NString NName;
		public readonly IntPtr Target;

#nullable enable
		public string? Name => NName.ToString();
#nullable disable
	}

	internal static class InteropInterface {
		internal readonly static Set<Type> cached_types = new();
		internal readonly static Set<MethodInfo> cached_methods = new();
		internal readonly static Set<FieldInfo> cached_fields = new();
		internal readonly static Set<PropertyInfo> cached_properties = new();
		internal readonly static Set<Attribute> cached_attributes = new();

		private static Dictionary<Type, ManagedType> type_converters = new() {
			{ typeof(sbyte), ManagedType.SByte },
			{ typeof(byte), ManagedType.Byte },
			{ typeof(short), ManagedType.Short },
			{ typeof(ushort), ManagedType.UShort },
			{ typeof(int), ManagedType.Int },
			{ typeof(uint), ManagedType.UInt },
			{ typeof(long), ManagedType.Long },
			{ typeof(ulong), ManagedType.ULong },
			{ typeof(float), ManagedType.Float },
			{ typeof(double), ManagedType.Double },
			{ typeof(bool), ManagedType.Bool },
			{ typeof(NBool32), ManagedType.Bool },
		};

		internal enum TypeAccessibility {
			Public,
			Private,
			Protected,
			Internal,
			ProtectedPublic,
			PrivateProtected
		}

		private static TypeAccessibility GetTypeAccessibility(FieldInfo finfo) {
			if (finfo.IsPublic) return TypeAccessibility.Public;
			if (finfo.IsPrivate) return TypeAccessibility.Private;
			if (finfo.IsFamily) return TypeAccessibility.Protected;
			if (finfo.IsAssembly) return TypeAccessibility.Internal;
			if (finfo.IsFamilyOrAssembly) return TypeAccessibility.ProtectedPublic;
			if (finfo.IsFamilyAndAssembly) return TypeAccessibility.PrivateProtected;
			return TypeAccessibility.Public;
		}

		private static TypeAccessibility GetTypeAccessibility(MethodInfo minfo) {
			if (minfo.IsPublic) return TypeAccessibility.Public;
			if (minfo.IsPrivate) return TypeAccessibility.Private;
			if (minfo.IsFamily) return TypeAccessibility.Protected;
			if (minfo.IsAssembly) return TypeAccessibility.Internal;
			if (minfo.IsFamilyOrAssembly) return TypeAccessibility.ProtectedPublic;
			if (minfo.IsFamilyAndAssembly) return TypeAccessibility.PrivateProtected;
			return TypeAccessibility.Public;
		}

#nullable enable
		internal static Type? FindType(string? name) {
			var type = Type.GetType(name!,
				(name) => AssemblyLoader.ResolveAssembly(null, name),
				(assembly,name,ignore) => {
					return assembly != null ? 
						assembly.GetType(name, false, ignore) : Type.GetType(name, false, ignore);
				}
			);

			return type;
		}

		internal static object? CreateInstance(Type type, params object?[]? args) {
			return type.Assembly.CreateInstance(type.FullName ?? string.Empty, false, 
																					BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance, 
																					null, args!, null, null);
		}

		internal static unsafe T? FindSuitableMethod<T>(string? method_name, ManagedType* param_type, Int32 argc, ReadOnlySpan<T> methods) where T : MethodBase {
			if (method_name == null) {
				return null;
			}

			foreach (var minfo in methods) {
				var parameters = minfo.GetParameters();
				if (parameters.Length != argc) {
					continue;
				}

				if (method_name == minfo.ToString()) {
					return minfo;
				}

				if (minfo.Name != method_name) {
					continue;
				}

				Int32 type_match = 0;
				for (Int32 i = 0; i < parameters.Length; i++) {
					ManagedType ptype;
					if (parameters[i].ParameterType.IsPointer || parameters[i].ParameterType == typeof(IntPtr)) {
						ptype = ManagedType.Pointer;
					} else if (!type_converters.TryGetValue(parameters[i].ParameterType, out ptype)) {
						ptype = ManagedType.Unknown;
					}

					if (ptype == param_type[i]) {
						type_match++;
					}
				}

				if (type_match == argc) {
					return minfo;
				}
			}

			return null;
		}
#nullable disable

		[UnmanagedCallersOnly]
		private static unsafe void GetAsmTypes(Int32 asm_id, Int32* out_types, Int32* out_type_count) {
			try {
				if (!AssemblyLoader.TryGetAssembly(asm_id, out var asm)) {
					LogMessage($"Couldn't get types for assembly '{asm_id}', assembly not found", MessageLevel.Error);
					return;
				}
				
				if (asm == null) {
					LogMessage($"Couldn't get types for assembly '{asm_id}', assembly is null", MessageLevel.Error);
					return;
				}

				ReadOnlySpan<Type> asm_types = asm.GetTypes();
				if (out_type_count != null) {
					*out_type_count = asm_types.Length;
				}

				if (out_types != null) {
					for (Int32 i = 0; i < asm_types.Length; i++) {
						out_types[i] = cached_types.Add(asm_types[i]);
					}
				}
			} catch (Exception ex) {
				HandleException(ex);
			}
		}

		[UnmanagedCallersOnly]
		private static unsafe void GetTypeId(NString name, Int32* out_type) {
			try {
				var type = FindType(name);
				if (type == null) {
					LogMessage($"Couldn't get type id for '{name}', type not found", MessageLevel.Error);
					return;
				}

				*out_type = cached_types.Add(type);
			} catch (Exception ex) {
				HandleException(ex);
			}
		}

		[UnmanagedCallersOnly]
		private static unsafe NString GetAsmQualifiedName(Int32 type) {
			try {
				if (!cached_types.TryGet(type, out var t)) {
					return NString.Null();
				}

				return t.AssemblyQualifiedName;
			} catch (Exception e) {
				HandleException(e);
				return NString.Null();
			}
		}

		[UnmanagedCallersOnly]
		private static unsafe NString GetFullTypeName(Int32 type_id) {
			try {
				if (!cached_types.TryGet(type_id, out var type)) {
					return NString.Null();
				}

				return type.FullName;
			} catch (Exception ex) {
				HandleException(ex);
				return NString.Null();
			}
		}

		[UnmanagedCallersOnly]
		private static unsafe void GetBaseType(Int32 in_type, Int32* out_base_type) {
			try {
				if (!cached_types.TryGet(in_type, out var type) || out_base_type == null) {
					return;
				}

				if (type.BaseType == null) {
					*out_base_type = 0;
					return;
				}

				*out_base_type = cached_types.Add(type.BaseType);
			} catch (Exception ex) {
				HandleException(ex);
			}
		}

		[UnmanagedCallersOnly]
		private static Int32 GetTypeSize(Int32 type) {
			try {
				if (!cached_types.TryGet(type, out var t)) {
					return 0;
				}

				return Marshal.SizeOf(t);
			} catch (Exception ex) {
				HandleException(ex);
				return -1;
			}
		}

		[UnmanagedCallersOnly]
		private static unsafe NBool32 IsTypeDerivedFrom(Int32 t0, Int32 t1) {
			try {
				if (!cached_types.TryGet(t0, out var type0) || !cached_types.TryGet(t1, out var type1)) {
					return false;
				}

				return type0.IsSubclassOf(type1);
			} catch (Exception ex) {
				HandleException(ex);
				return false;
			}
		}

		[UnmanagedCallersOnly]
		private static unsafe NBool32 IsAssignableTo(Int32 t0, Int32 t1) {
			try {
				if (!cached_types.TryGet(t0, out var type0) || !cached_types.TryGet(t1, out var type1)) {
					return false;
				}

				return type0.IsAssignableTo(type1);
			} catch (Exception ex) {
				HandleException(ex);
				return false;
			}
		}

		[UnmanagedCallersOnly]
		private static unsafe NBool32 IsAssignableFrom(Int32 t0, Int32 t1) {
			try {
				if (!cached_types.TryGet(t0, out var type0) || !cached_types.TryGet(t1, out var type1)) {
					return false;
				}

				return type0.IsAssignableFrom(type1);
			} catch (Exception ex) {
				HandleException(ex);
				return false;
			}
		}

		[UnmanagedCallersOnly]
		private static unsafe NBool32 IsSzArray(Int32 type_id) {
			try {
				if (!cached_types.TryGet(type_id, out var type)) {
					return false;
				}

				return type.IsSZArray;
			} catch (Exception ex) {
				HandleException(ex);
				return false;
			}
		}

		[UnmanagedCallersOnly]
		private static unsafe void GetElementType(Int32 id, Int32* out_type) {
			try {
				if (!cached_types.TryGet(id, out var type)) {
					return;
				}

				var elt_type = type.GetElementType();

				if (elt_type == null) {
					*out_type = 0;
				}

				*out_type = cached_types.Add(elt_type);
			} catch (Exception e) {
				HandleException(e);
			}
		}

		[UnmanagedCallersOnly]
		private static unsafe void GetTypeMethods(Int32 type, Int32* method_arr, Int32* count) {
			try {
				if (!cached_types.TryGet(type, out var t)) {
					return;
				}

				ReadOnlySpan<MethodInfo> methods = t.GetMethods(BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance | BindingFlags.Static);
				if (methods == null || methods.Length == 0) {
					*count = 0;
					return;
				}
				LogMessage($"Found {methods.Length} methods for type {t.FullName}", MessageLevel.Trace);

				*count = methods.Length;
				if (method_arr == null) {
					return;
				}

				for (Int32 i = 0; i < methods.Length; i++) {
					LogMessage($"  > Adding method {methods[i].Name} to cache", MessageLevel.Trace);
					method_arr[i] = cached_methods.Add(methods[i]);
				}

				LogMessage($"  > Added {methods.Length} methods to cache", MessageLevel.Trace);
			} catch (Exception ex) {
				HandleException(ex);
			}
		}

		[UnmanagedCallersOnly]
		private static unsafe void GetTypeFields(Int32 type, Int32* field_arr, Int32* field_count) {
			try {
				if (!cached_types.TryGet(type, out var t)) {
					return;
				}

				ReadOnlySpan<FieldInfo> fields = t.GetFields(BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance | BindingFlags.Static);
				if (fields == null || fields.Length == 0) {
					*field_count = 0;
					return;
				}

				*field_count = fields.Length;

				if (field_arr == null) {
					return;
				}

				for (Int32 i = 0; i < fields.Length; i++) {
					field_arr[i] = cached_fields.Add(fields[i]);
				}
			} catch (Exception ex) {
				HandleException(ex);
			}
		}

		[UnmanagedCallersOnly]
		private static unsafe void GetTypeProperties(Int32 type, Int32* arr, Int32* count) {
			try {
				if (!cached_types.TryGet(type, out var t)) {
					return;
				}

				ReadOnlySpan<PropertyInfo> properties = t.GetProperties(BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance | BindingFlags.Static);
				if (properties == null || properties.Length == 0) {
					*count = 0;
					return;
				}

				*count = properties.Length;

				if (arr == null) {
					return;
				}

				for (Int32 i = 0; i < properties.Length; i++) {
					arr[i] = cached_properties.Add(properties[i]);
				}
			} catch (Exception ex) {
				HandleException(ex);
			}
		}

		[UnmanagedCallersOnly]
		private static unsafe NBool32 HasAttribute(Int32 type , Int32 attr_type) {
			try {
				if (!cached_types.TryGet(type, out var t) || !cached_attributes.TryGet(attr_type, out var attr)) {
					return false;
				}

				return t.GetCustomAttribute(attr) != null;
			} catch (Exception ex) {
				HandleException(ex);
				return false;
			}
		}

		[UnmanagedCallersOnly]
		private static unsafe void GetAttributes(Int32 type, Int32* attributes, Int32* count) {
			try {
				if (!cached_types.TryGet(type, out var t)) {
					return;
				}

				var attrs = t.GetCustomAttributes().ToImmutableArray();
				if (attrs == null || attrs.Length == 0) {
					*count = 0;
					return;
				}

				*count = attrs.Length;

				if (attributes == null) {
					return;
				}

				for (Int32 i = 0; i < attrs.Length; i++) {
					attributes[i] = cached_attributes.Add(attrs[i]);
				}
			} catch (Exception ex) {
				HandleException(ex);
			}
		}

		[UnmanagedCallersOnly]
		private static unsafe ManagedType GetTypeManagedType(Int32 type) {
			try {
				if (!cached_types.TryGet(type, out var t)) {
					return ManagedType.Unknown;
				}

				if (!type_converters.TryGetValue(t, out var mtype)) {
					mtype = ManagedType.Unknown;
				}

				return mtype;
			} catch (Exception ex) {
				HandleException(ex);
				return ManagedType.Unknown;
			}
		}

		[UnmanagedCallersOnly]
		private static unsafe NString GetMethodName(Int32 method_info) {
			try {
				if (!cached_methods.TryGet(method_info, out var minfo)) {
					return NString.Null();
				}

				return minfo.Name;
			} catch (Exception ex) {
				HandleException(ex);
				return NString.Null();
			}
		}

		[UnmanagedCallersOnly]
		private static unsafe void GetMethodReturnType(Int32 method_info, Int32* out_type) {
			try {
				if (!cached_methods.TryGet(method_info, out var minfo) || out_type == null)
					return;

				*out_type = cached_types.Add(minfo.ReturnType);
			} catch (Exception ex) {
				HandleException(ex);
			}
		}

		[UnmanagedCallersOnly]
		private static unsafe void GetMethodParameterTypes(Int32 minfo, Int32* out_param_types, Int32* count) {
			try {
				if (!cached_methods.TryGet(minfo, out var methodInfo)) {
					return;
				}

				ReadOnlySpan<ParameterInfo> parameters = methodInfo.GetParameters();

				if (parameters == null || parameters.Length == 0) {
					*count = 0;
					return;
				}

				*count = parameters.Length;

				if (out_param_types == null) {
					return;
				}

				for (Int32 i = 0; i < parameters.Length; i++) {
					out_param_types[i] = cached_types.Add(parameters[i].ParameterType);
				}
			} catch (Exception e) {
				HandleException(e);
			}
		}

		[UnmanagedCallersOnly]
		private static unsafe void GetMethodAttributes(Int32 minfo, Int32* out_attrs, Int32* count) {
			try {
				if (!cached_methods.TryGet(minfo, out var methodInfo)) {
					*count = 0;
					return;
				}

				var attributes = methodInfo.GetCustomAttributes().ToImmutableArray();

				if (attributes.Length == 0) {
					*count = 0;
					return;
				}

				*count = attributes.Length;

				if (out_attrs == null) {
					return;
				}

				for (Int32 i = 0; i < attributes.Length; i++) {
					out_attrs[i] = cached_attributes.Add(attributes[i]);
				}
			} catch (Exception ex) {
				HandleException(ex);
			}
		} 

		[UnmanagedCallersOnly]
		private static unsafe TypeAccessibility GetMethodAccessibility(Int32 id) {
			try {
				if (!cached_methods.TryGet(id, out var minfo)) {
					return TypeAccessibility.Internal;
				}

				return GetTypeAccessibility(minfo);
			} catch (Exception ex) {
				HandleException(ex);
				return TypeAccessibility.Public;
			}
		}

		[UnmanagedCallersOnly]
		private static unsafe NString GetFieldName(Int32 id, Int32* out_type) {
			try {
				if (!cached_fields.TryGet(id, out var finfo)) {
					return NString.Null();
				}

				return finfo.Name;
			} catch (Exception ex) {
				HandleException(ex);
				return NString.Null();
			}
		}

		[UnmanagedCallersOnly]
		private static unsafe void GetFieldType(Int32 id, Int32* out_type) {
			try {
				if (!cached_fields.TryGet(id, out var finfo) || out_type == null) {
					return;
				}

				*out_type = cached_types.Add(finfo.FieldType);
			} catch (Exception ex) {
				HandleException(ex);
			}
		}

		[UnmanagedCallersOnly]
		private static unsafe TypeAccessibility GetFieldAccessibility(Int32 id) {
			try {
				if (!cached_fields.TryGet(id, out var finfo)) {
					return TypeAccessibility.Internal;
				}

				return GetTypeAccessibility(finfo);
			} catch (Exception ex) {
				HandleException(ex);
				return TypeAccessibility.Public;
			}
		}

		[UnmanagedCallersOnly]
		private static unsafe void GetFieldAttributes(Int32 id, Int32* out_attrs, Int32* count) {
			try {
				if (!cached_fields.TryGet(id, out var finfo)) {
					*count = 0;
					return;
				}

				var attributes = finfo.GetCustomAttributes().ToImmutableArray();

				if (attributes.Length == 0) {
					*count = 0;
					return;
				}

				*count = attributes.Length;

				if (out_attrs == null) {
					return;
				}

				for (Int32 i = 0; i < attributes.Length; i++) {
					out_attrs[i] = cached_attributes.Add(attributes[i]);
				}
			} catch (Exception ex) {
				HandleException(ex);
			}
		}

		[UnmanagedCallersOnly]
		private static unsafe NString GetPropertyName(Int32 id, Int32* out_type) {
			try {
				if (!cached_properties.TryGet(id, out var pinfo)) {
					return NString.Null();
				}

				return pinfo.Name;
			} catch (Exception ex) {
				HandleException(ex);
				return NString.Null();
			}
		}

		[UnmanagedCallersOnly]
		private static unsafe void GetPropertyType(Int32 id, Int32* out_type) {
			try {
				if (!cached_properties.TryGet(id, out var pinfo) || out_type == null) {
					return;
				}

				*out_type = cached_types.Add(pinfo.PropertyType);
			} catch (Exception ex) {
				HandleException(ex);
			}
		}

		[UnmanagedCallersOnly]
		private static unsafe void GetPropertyAttributes(Int32 id, Int32* out_attrs, Int32* count) {
			try {
				if (!cached_properties.TryGet(id, out var pinfo)) {
					*count = 0;
					return;
				}

				var attributes = pinfo.GetCustomAttributes().ToImmutableArray();

				if (attributes.Length == 0) {
					*count = 0;
					return;
				}

				*count = attributes.Length;

				if (out_attrs == null) {
					return;
				}

				for (Int32 i = 0; i < attributes.Length; i++) {
					out_attrs[i] = cached_attributes.Add(attributes[i]);
				}
			} catch (Exception ex) {
				HandleException(ex);
			}
		}

		[UnmanagedCallersOnly]
		private static unsafe void GetAttributeValue(Int32 attr, NString name, IntPtr out_val) {
			try {
				if (!cached_attributes.TryGet(attr, out var attribute)) {
					return;
				}

				var target = attribute.GetType();
				var field = target.GetField(name!, BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance);
				if (field == null) {
					LogMessage($"Failed to find field with name '{name}' in attribute {target.FullName}", MessageLevel.Error);
					return;
				}

				Interop.DotOtherMarshal.MarshalReturn(field.GetValue(attr), field.FieldType, out_val);
			} catch (Exception ex) {
				HandleException(ex);
			}
		}

		[UnmanagedCallersOnly]
		private static unsafe void GetAttributeType(Int32 attr, Int32* out_type) {
			try { 
				if (!cached_attributes.TryGet(attr, out var attribute)) {
					return;
				}

				*out_type = cached_types.Add(attr.GetType());
			} catch (Exception e) {
				HandleException(e);
			}
		}
	}

}