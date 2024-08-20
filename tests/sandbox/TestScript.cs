namespace Other {

  public class TestScript : OtherObject {
    public float speed = 1.0f;
    public override void Update(float dt) {
      Transform.Rotate(0.1f , new Vec3(0 , 1.0f , 0)); 
    }
  }

}