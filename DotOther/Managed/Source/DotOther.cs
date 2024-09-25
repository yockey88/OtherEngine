using System;
using DotOther.Managed;

namespace DotOther {

  using static DotOtherHost;

  public class Host {
    public static NObject GetNativeObject(UInt64 internal_id) {
      try {
        return DotOtherHost.GetNativeObject(internal_id);
      } catch (Exception e) {
        DotOtherHost.HandleException(e);
        return null;
      }
    }
  }

}