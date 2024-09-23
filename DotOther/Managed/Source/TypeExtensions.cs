using System;
using System.Reflection;
using System.Linq;
using System.Runtime.CompilerServices;

namespace DotOther.Managed {

  public static class ExtensionMethods {
    public static T GetCustomAttribute<T>(this Type type, T attr) where T : Attribute {
      object[] attrs = type.GetCustomAttributes(false);
      return attrs.OfType<T>().FirstOrDefault();
    }
  }

}