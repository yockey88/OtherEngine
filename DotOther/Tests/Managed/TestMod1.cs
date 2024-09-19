using System;

namespace DotOther.Tests {

  class Mod1 {
    private Int32 my_num;

    public Int32 MyNum {
      get => my_num;
      set => my_num = value;
    }

    public Mod1() {
      my_num = 0;
    }

    public Mod1(Int32 num) {
      my_num = num;
    }

    public void Test() {
      Console.WriteLine("Mod1.Test");
    }
  }


}