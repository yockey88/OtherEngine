using System;
using Other;

namespace Yockcraft {
  
  public class CameraController : OtherObject {
    float speed = 0.5f;
    Vec3 position = new Vec3(0, 0, 0);

    private Camera camera;

    public override void OnInitialize() {
      camera = GetComponent<Camera>();
    }
  }

}