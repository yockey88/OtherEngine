using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;

namespace DotOther.Managed {

  public class Set<T> {
    private readonly Dictionary<Int32, T> elements = new();

    public bool Contains(Int32 id) {
      return elements.ContainsKey(id);
    }

    public Int32 Size {
      get {
        return elements.Count;
      }
    }

    public IEnumerable<T> Elements {
      get {
        return elements.Values;
      }
    }

#nullable enable
    public Int32 Add(T? element) {
      if (element == null) {
        throw new ArgumentNullException(nameof(element));
      }

      Int32 hash = RuntimeHelpers.GetHashCode(element);
      _ = elements.TryAdd(hash, element);
      return hash;
    }

    public bool TryGet(Int32 id, out T? element) {
      return elements.TryGetValue(id, out element);
    }

    public void Clear() {
      elements.Clear();
    }
#nullable disable
  }

}