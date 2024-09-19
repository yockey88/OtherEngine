using System;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics.CodeAnalysis;
using System.Runtime.InteropServices;

namespace DotOther.Managed {

  [StructLayout(LayoutKind.Sequential, Pack = 1)]
  public struct NBool32 {
    public uint Value { 
      get; 
      set; 
    }

    public static implicit operator NBool32(bool val) => new() { Value = val ? 1u : 0u };
    public static implicit operator bool(NBool32 b) => b.Value > 0;
  }

  public sealed class NArrayEnumerator<T> : IEnumerator<T> {
    private readonly T[] array;
    private Int32 index;

    public NArrayEnumerator(T[] array) {
      this.array = array;
      index = -1;
    }

    public void Dispose() {
      index = -1;
      GC.SuppressFinalize(this);
    }

    public bool MoveNext() => ++index < array.Length;
    public void Reset() => index = -1;

    void IEnumerator.Reset() => index = -1;
    void IDisposable.Dispose() {
      index = -1;
      GC.SuppressFinalize(this);
    }
    object IEnumerator.Current => Current!;
    public T Current => array[index];
  }
  
  [StructLayout(LayoutKind.Sequential, Pack = 1)]
  public sealed class NArray<T> : IEnumerable<T> {
    private readonly IntPtr array;
    private readonly Int32 length;
    private NBool32 disposed;

    public Int32 Length => length;

    public NArray(IntPtr len) {
      this.array = Marshal.AllocHGlobal(len * Marshal.SizeOf<T>());
      this.length = length;
    }

    public NArray([DisallowNull] T?[] array) {
      this.array = Marshal.AllocHGlobal(array.Length * Marshal.SizeOf<T>());
      this.length = array.Length;

      for (int i = 0; i < array.Length; i++) {
        var elem = array[i];
        if (elem == null) {
          continue;
        }

        Marshal.StructureToPtr(array[i], IntPtr.Add(this.array, i * Marshal.SizeOf<T>()), false);
      }
    }

    internal NArray(IntPtr array, int length) {
      this.array = array;
      this.length = length;
    }

    public T[] ToArray() {
      Span<T> data = Span<T>.Empty;
      if (array != IntPtr.Zero && length > 0) {
        unsafe {
          data = new Span<T>(array.ToPointer(), length);
        }
      }
      return data.ToArray();
    }

    public Span<T> ToSpan() {
      unsafe {
        return new Span<T>(array.ToPointer(), length);
      }
    }

    public ReadOnlySpan<T> ToReadOnlySpan() => ToSpan();

    public void Dispose() {
      if (!disposed) {
        if (array != IntPtr.Zero) {
          Marshal.FreeHGlobal(array);
        }

        disposed = true;
      }

      GC.SuppressFinalize(this);
    }

    public IEnumerator<T> GetEnumerator() => new NArrayEnumerator<T>(this);
    IEnumerator IEnumerable.GetEnumerator() => new NArrayEnumerator<T>(this);

    public T? this[Int32 index] {
      get => Marshal.PtrToStructure<T>(IntPtr.Add(array, index * Marshal.SizeOf<T>()));
      set => Marshal.StructureToPtr<T>(value!, IntPtr.Add(array, index * Marshal.SizeOf<T>()), false);
    }

    public static implicit operator T[](NArray<T> array) => array.ToArray();
  }
  
  [StructLayout(LayoutKind.Sequential)]
  public struct NString : IDisposable {
    internal IntPtr native_string;
    private NBool32 disposed;

    public void Dispose() {
      if (!disposed) {
        if (native_string != IntPtr.Zero) {
          Marshal.FreeCoTaskMem(native_string);
          native_string = IntPtr.Zero;
        }

        disposed = true;
      }

      GC.SuppressFinalize(this);
    }

    public override string? ToString() => this;

    public static NString Null() => new NString(){ native_string = IntPtr.Zero };

    public static implicit operator NString(string? str) => new(){ native_string = Marshal.StringToCoTaskMemAuto(str) };
    public static implicit operator string?(NString str) => Marshal.PtrToStringAuto(str.native_string);
  }

  [StructLayout(LayoutKind.Sequential, Pack = 1)]
  public struct NInstance<T> {
    private readonly IntPtr instance;
    private readonly IntPtr unused;

    public T? Get() {
      if (instance == IntPtr.Zero) {
        return default;
      }

      GCHandle handle = GCHandle.FromIntPtr(instance);
      if (!(handle.Target is T)) {
        return default;
      }

      return (T)handle.Target;
    }

    public static implicit operator T?(NInstance<T> instance) => instance.Get();
  }

  [StructLayout(LayoutKind.Sequential, Pack = 1)]
  public struct ReflectionType {
    private readonly Int32 type_id;
    public Int32 ID => type_id;

    public ReflectionType(Int32 id) {
      type_id = id;
    }

#nullable enable
    // public static implicit operator ReflectionType(Type? type) => new(InteropInterface.CachedTypes.Add(type));
#nullable disable
  }

}