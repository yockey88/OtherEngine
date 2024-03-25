using System;

namespace Other {

  public class Entity {
    public Entity() {}
  
    public virtual void Initialize() {
      Console.WriteLine("Entity Initialize");
    }
    public virtual void Update(float dt) {
    }
    public virtual void Render() {}
    public virtual void RenderUI() {}
    public virtual void Shutdown() {
      Console.WriteLine("Entity Shutdown");
    }
  }

}
