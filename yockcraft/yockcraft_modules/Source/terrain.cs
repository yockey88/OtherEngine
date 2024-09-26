using System;
using System.Globalization;
using Other;

namespace Yockcraft {
  public class Terrain : OtherObject {
    public enum DrawMode {
      NOISE_MAP,
      COLOR_MAP,
      TERRAIN_MESH
    }

    UUID cube_handle;

    private StaticMesh mesh;

    public override void OnInitialize() {
      Vec3 scale = new Vec3(1, 1, 1);
      // cube_handle = ModelFactory.CreateBox(ref scale);

      // mesh = CreateComponent<StaticMesh>();
      // mesh.MeshHandle = cube_handle;
    }

    public override void Update(float dt) {
      // Transform.Rotate(0.1f , new Vec3(0 , 1.0f , 0)); 
    }
  }

}
