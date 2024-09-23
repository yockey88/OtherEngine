using System;
using DotOther;
using DotOther.Managed.Interop;

namespace DotOther.Tests {

  public class Mod1 {
    internal static unsafe delegate*<IntPtr> GetNativeObject;

    private Int32 my_num;

    public Int32 MyNum {
      get => my_num;
      set => my_num = value;
    }

    public float number = 0.0f;

    public Mod1() {
      Console.WriteLine($" Mod1 Asm Name : {this.GetType().AssemblyQualifiedName}");
      my_num = 0;
    }

    public void Test() {
      Console.WriteLine("Mod1.Test");
    }

    public void Test(Int32 num) {
      Console.WriteLine("Mod1.Test: " + num);
    }

    public unsafe void TestInternalCall() {
      try {
        if (GetNativeObject == null) {
          Console.WriteLine("GetNativeObject is null");
          return;
        }

        NObject obj = DotOtherMarshal.MarshalPointer<NObject>(GetNativeObject());
        obj.Invoke("Test");
        
      } catch (Exception e) {
        Console.WriteLine($"Exception in TestInternalCall: {e.Message} |\n{e.StackTrace}");
      }
    }
  }


}