using System;
using Other;

namespace Forest {

  class CharacterController : OtherObject {

    Camera cam = null;

    public override void OnStart() {
      cam = GetComponent<Camera>();
      if (cam != null) {
        Logger.WriteDebug("Camera found");
        Logger.WriteDebug($"Camera.Position = ${cam.Position}");
      }
      // Logger.WriteDebug($"Camera.Position = ${cam.Position}");
    }

    public override void Update(float dt) {
    }

    public override void OnContact(OtherObject other) {
    }
  }

}