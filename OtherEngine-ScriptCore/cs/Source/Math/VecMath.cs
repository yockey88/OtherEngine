using System.Linq.Expressions;
using System.Runtime.CompilerServices;

namespace Other {

  public class VecMath {
    static public Quaternion AngleAxis(float angle , Vec3 axis) {
      float s = Mathf.Sin(angle * 0.5f);
      return new Quaternion(Mathf.Cos(angle * 0.5f), axis * s);
    }
  }

}