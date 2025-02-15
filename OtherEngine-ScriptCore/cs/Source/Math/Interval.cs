namespace Other {

  public class Interval {
    private float min;
    private float max;

    private static Interval empty = new Interval(Mathf.PositiveInfinity, Mathf.NegativeInfinity);
    private static Interval universe = new Interval(Mathf.NegativeInfinity, Mathf.PositiveInfinity);
    
    public Interval(float min, float max) {
      this.min = min;
      this.max = max;
    }

    public bool Contains(float value) {
      return value >= min && value <= max;
    }

    public float ClampToClosestBound(float value) {
      float min_diff = Mathf.Abs(value - min);
      float max_diff = Mathf.Abs(value - max);
      return min_diff < max_diff ? min : max;
    }

    public float Min {
      get { return min; }
      set { min = value; }
    }

    public float Max {
      get { return max; }
      set { max = value; }
    }

    public static Interval Empty {
      get { return empty; }
    }

    public static Interval Universe {
      get { return universe; }
    }
  }

}