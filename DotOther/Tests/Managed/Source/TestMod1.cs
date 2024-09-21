using System;
using System.Runtime.CompilerServices;
using DotOther;

namespace DotOther.Tests {

  public class Mod1 {
    
    internal unsafe delegate NObject GetNativeObject();

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
  }


}