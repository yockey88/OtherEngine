using System;

namespace Other {

  public enum PhysicsType {
    Physics2D ,
    Physics3D
  }

  public enum PhysicsBodyType : UInt32 {
    Static = 0,
    Dynamic ,
    Kinematic
  }

}