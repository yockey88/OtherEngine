using System;
using DotOther;
using DotOther.Managed;
using DotOther.Managed.Interop;

namespace DotOther.Tests {

  public class Mod1 : OtherObject {
    private static unsafe delegate*<NObject> GetNativeObject;

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
      NObject obj = GetNativeObject();
      if (obj == null) {
        Console.WriteLine("Failed to get native object");
        return;
      }
      
      obj.Invoke("Test");
    }
  }


}