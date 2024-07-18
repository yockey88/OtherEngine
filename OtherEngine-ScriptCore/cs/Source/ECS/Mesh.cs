namespace Other {

  public class Mesh : Component {
    public ulong asset_handle;

    public Mesh(OtherObject obj) : base(obj) {
      asset_handle = 0;
    }
  }

}