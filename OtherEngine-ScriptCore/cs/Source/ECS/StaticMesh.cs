namespace Other {

  public class StaticMesh : Component {
    public StaticMesh() {
    }

    public StaticMesh(OtherObject obj) : base(obj) {
    }

    // public UUID MeshHandle {
    //   get => Scene.GetStaticMeshHandle(Object.ObjectID); 
    //   set => Scene.SetStaticMeshHandle(Object.ObjectID , value);
    // }

    // public Material Material {
    //   get {
    //     Scene.GetStaticMeshMaterial(Object.ObjectID , out Material material);
    //     return material;
    //   }
    //   set => Scene.SetStaticMeshMaterial(Object.ObjectID , ref value);
    // }
  }

}