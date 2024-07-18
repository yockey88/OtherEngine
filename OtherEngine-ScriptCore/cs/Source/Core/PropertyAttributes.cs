using System;

namespace Other {

  public class OtherAttribute : Attribute {}

  [AttributeUsage(AttributeTargets.Field | AttributeTargets.Property , AllowMultiple = false)]
  public class BoundsAttribute : OtherAttribute {

    private double min;
    private double max;

    public double Min { get => min; }
    public double Max { get => max; }

    public BoundsAttribute(int min, int max) {
      this.min = Math.Min(min, max);
      this.max = Math.Max(min, max);
    }

    public BoundsAttribute(double min, double max) {
      this.min = Math.Min(min, max);
      this.max = Math.Max(min, max);
    }
  }

}