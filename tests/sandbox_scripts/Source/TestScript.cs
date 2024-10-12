using System;
using System.Collections.Generic;
using System.Reflection.Metadata;

namespace Other {

  public class TestScript : OtherObject {
    public void Method() {
      Console.WriteLine("Hello from TestScript!");
    }

    public override void OnBehaviorLoad() {
      Console.WriteLine("TestScript.OnBehaviorLoad");
    }

    public override void OnBehaviorUnload() {
      Console.WriteLine("TestScript.OnBehaviorUnload");
    }

    public override void OnInitialize() {
      Console.WriteLine("TestScript.OnInitialize");
    }

    public override void OnShutdown() {
      Console.WriteLine("TestScript.OnShutdown");
    }
    
    public override void OnStart() {
      Console.WriteLine("TestScript.OnStart");
    }

    public override void OnStop() {
      Console.WriteLine("TestScript.OnStop");
    }

    public override void EarlyUpdate(float dt) {
      Console.WriteLine($"TestScript.OnEarlyUpdate({dt})");
    }

    public override void Update(float dt) {
      Console.WriteLine($"TestScript.OnUpdate({dt})");

      Transform transform = GetComponent<Transform>();
      if (transform == null) {
        Console.WriteLine(" > Transform is null");
        return;
      }

      Console.WriteLine($" This Name : {Name}");
      Console.WriteLine($" > Transform.Position: {transform.Position.ToString()}");
      Console.WriteLine($" > Transform.Scale: {transform.Scale.ToString()}");
      Console.WriteLine($" > Transform.Rotation: {transform.Rotation.ToString()}");
      transform.Position += new Vec3(1.1f, 2.2f, 3.3f);
      transform.Scale += new Vec3(3.3f , 2.2f , 1.1f);
      transform.Rotation += new Vec3(1.2f , 3.4f , 5.6f);
      Console.WriteLine($" > Transform.Position: {transform.Position.ToString()}");
      Console.WriteLine($" > Transform.Scale: {transform.Scale.ToString()}");
      Console.WriteLine($" > Transform.Rotation: {transform.Rotation.ToString()}");

      Relationship relationship = GetComponent<Relationship>();
      if (relationship == null) {
        Console.WriteLine(" > Relationship is null");
        return;
      }

      List<UUID> ids = relationship.Children;
      Console.WriteLine($" > Relationship.Children: {ids.Count}");

      UUID child_id = ids[0];
      if (!Scene.IsValidHandle(child_id)) {
        Console.WriteLine(" > Invalid UUID");
        return;
      }

      EntityHandle child = Scene.GetEntityHandle(child_id);
      if (child == null) {
        Console.WriteLine(" > EntityHandle is null");
        return;
      }

      OtherObject obj = child;
      Console.WriteLine($" > OtherObject.Name: {obj.Name}");
      
      var t = obj.GetComponent<Transform>();
      t.Rotate(0.1f , new Vec3(1.0f , 1.0f , 1.0f));

      Console.WriteLine($" > Transform.Position: {t.Position.ToString()}");
      Console.WriteLine($" > Transform.Position: {t.Rotation.ToString()}");
    }

    public override void LateUpdate(float dt) {
      Console.WriteLine($"TestScript.OnLateUpdate({dt})");
    }

    public override void Render() {
      Console.WriteLine("TestScript.OnRender");
    }

    public override void RenderUI() {
      Console.WriteLine("TestScript.OnRenderUI");
    }
  }
}
