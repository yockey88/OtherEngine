using System;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;

using DotOther.Managed;

namespace DotOther.Managed.Interop {

  public static class DotOtherMarshal {
    struct ArrayContainer {
      public IntPtr Data;
      public Int32 Length;
    }

#nullable enable
    public static void MarshalReturn(object? val, Type? type, IntPtr result) {
#nullable disable
      if (type == null) {
        return;
      }

      if (type.IsSZArray) {
        var array = val as Array;
        var element_type = type.GetElementType();
        CopyArrayToBuffer(result, array, element_type);
      } else if (val is string str) {
        NString nstring = str;
        Marshal.StructureToPtr(nstring, result, false);
      } else if (val is NString nstring) {
        Marshal.StructureToPtr(nstring, result, false);
      } else if (type.IsPointer) {
        unsafe {
          if (val == null) {
            Marshal.WriteIntPtr(result, IntPtr.Zero);
          } else {
            Marshal.WriteIntPtr(result, (IntPtr)val);
          }
        }
      } else {
        var val_size = Marshal.SizeOf(type);
        var handle = GCHandle.Alloc(val, GCHandleType.Pinned);

        unsafe {
          Buffer.MemoryCopy(handle.AddrOfPinnedObject().ToPointer(), result.ToPointer(), val_size, val_size);
        }

        handle.Free();
      }
    }

    struct ArrayObject {
      public IntPtr handle;
      public Int32 padding;
    }

    public static object? MarshalArray(IntPtr arr, Type? elt_type) {
      if (elt_type == null) {
        return null;
      }

      var arr_cont = MarshalPointer<ArrayContainer>(arr);
      var elts = Array.CreateInstance(elt_type, arr_cont.Length);

      if (elt_type.IsValueType) {
        Int32 elt_size = Marshal.SizeOf(elt_type);
        
        for (int i = 0; i < arr_cont.Length; i++) {
          unsafe {
            IntPtr src = (IntPtr)(((byte*)arr_cont.Data.ToPointer()) + (i * elt_size));
            elts.SetValue(Marshal.PtrToStructure(src, elt_type), i);
          }
        }
      } else {
        for (int i = 0; i < arr_cont.Length; i++) {
          unsafe {
            IntPtr src = (IntPtr)(((byte*)arr_cont.Data.ToPointer()) + (i * Marshal.SizeOf<ArrayObject>()));
            var managed_obj = MarshalPointer<ArrayObject>(src);
            var target = GCHandle.FromIntPtr(managed_obj.handle).Target;
            elts.SetValue(target, i);
          }
        }
      }

      return elts;
    }
    
    public static void CopyArrayToBuffer(IntPtr dest, Array? arr, Type? elt_type) {
      if (arr == null || elt_type == null) {
        return;
      }

      var elt_size = Marshal.SizeOf(elt_type);
      int blen = arr.Length * elt_size;
      var mem = Marshal.AllocHGlobal(blen);

      Int32 offset = 0;

      foreach (var e in arr) {
        var elt_handle = GCHandle.Alloc(mem, GCHandleType.Pinned);

        unsafe {
          Buffer.MemoryCopy(elt_handle.AddrOfPinnedObject().ToPointer(), (((byte*)mem.ToPointer()) + offset), elt_size, elt_size);
        }

        offset += elt_size;
        elt_handle.Free();
      }

      ArrayContainer c = new ArrayContainer {
        Data = mem,
        Length = arr.Length
      };

      var handle = GCHandle.Alloc(c, GCHandleType.Pinned);

      unsafe {
        Buffer.MemoryCopy(handle.AddrOfPinnedObject().ToPointer(), dest.ToPointer(), Marshal.SizeOf<ArrayContainer>(), Marshal.SizeOf<ArrayContainer>());
      }

      handle.Free();
    }

    public static object? MarshalPointer(IntPtr ptr, Type type) {
      if (type.IsPointer || type == typeof(IntPtr)) {
        return ptr;
      }

      if (type == typeof(bool)) {
        return Marshal.PtrToStructure<byte>(ptr) != 0;
      }

      if (type == typeof(string)) {
        var nstring = Marshal.PtrToStructure<NString>(ptr);
        return nstring.ToString();
      }

      if (type == typeof(NString)) {
        return Marshal.PtrToStructure<NString>(ptr);
      }

      if (type.IsSZArray) {
        return MarshalArray(ptr, type.GetElementType());
      }

      if (type.IsGenericType) {
        if (type == typeof(NArray<>).MakeGenericType(type.GetGenericArguments().First())) {
          var elts = Marshal.ReadIntPtr(ptr, 0);
          var elt_count = Marshal.ReadInt32(ptr, Marshal.SizeOf<IntPtr>());
          var gen_type = typeof(NArray<>).MakeGenericType(type.GetGenericArguments().First());
          throw new Exception("Not implemented");
          // return TypeInterface.CreateInstance(gen_type, elts, elt_count);
        }
      }

      if (type.IsClass) {
        var handle_ptr = Marshal.ReadIntPtr(ptr);
        var handle = GCHandle.FromIntPtr(handle_ptr);
        return handle.Target;
      }

      return Marshal.PtrToStructure(ptr, type);
    }

    public static T? MarshalPointer<T>(IntPtr ptr) => Marshal.PtrToStructure<T>(ptr);

    public static IntPtr[] NArrayToIntPtrArray(IntPtr narr , Int32 len) {
      try {
        if (narr == IntPtr.Zero || len == 0) {
          return Array.Empty<IntPtr>();
        }

        IntPtr[] arr = new IntPtr[len];
        for (int i = 0; i < len; i++) {
          arr[i] = Marshal.ReadIntPtr(narr, i * Marshal.SizeOf<nint>());
        }

        return arr;
      } catch (Exception ex) {
        DotOtherHost.HandleException(ex);
        return Array.Empty<IntPtr>();
      }
    }

    public static object?[]? MarshalParameterArray(IntPtr narr, Int32 len, MethodBase? method_info) {
      if (method_info == null) {
        return null;
      }

      if (narr == IntPtr.Zero || len == 0) {
        return null;
      }

      var infos = method_info.GetParameters();
      var pointers = NArrayToIntPtrArray(narr, len);
      var res = new object?[len];

      for (Int32 i = 0; i < pointers.Length; i++) {
        res[i] = MarshalPointer(pointers[i], infos[i].ParameterType);
      }

      return res;
    }
  }

}