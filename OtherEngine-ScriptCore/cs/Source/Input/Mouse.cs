using System.Runtime.CompilerServices;

namespace Other {

  public enum MouseButton : int {
    FIRST = 0 ,
    LEFT = FIRST ,
    MIDDLE = 1 , 
    RIGHT = 2 ,
    X1 = 3 ,
    X2 = 4 ,
    LAST = X2
  }

  public struct MouseState {
    private Vec2 position;
    private Vec2 last_position;
    bool in_window;
    
    public Vec2 Position {
      get => position;
      set {
        last_position = position;
        position = value;
      }
    }
    
    public Vec2 LastPosition {
      get => last_position;
      set => last_position = value;
    }
  }
  
  public static class Mouse {
    private const int NumButtons = 5;

    // [MethodImpl(MethodImplOptions.InternalCall)]
    // public static extern void SnapToCenter();

    // [MethodImpl(MethodImplOptions.InternalCall)]
    // public static extern void FreeCursor();
    
    // [MethodImpl(MethodImplOptions.InternalCall)]
    // public static extern void LockCursor();
    
    // [MethodImpl(MethodImplOptions.InternalCall)]
    // public static extern int GetX();

    // [MethodImpl(MethodImplOptions.InternalCall)]
    // public static extern int GetY();
    
    // [MethodImpl(MethodImplOptions.InternalCall)]
    // public static extern int PreviosX();

    // [MethodImpl(MethodImplOptions.InternalCall)]
    // public static extern int PreviousY();
    
    // [MethodImpl(MethodImplOptions.InternalCall)]
    // public static extern int GetDX();

    // [MethodImpl(MethodImplOptions.InternalCall)]
    // public static extern int GetDY();
    
    // [MethodImpl(MethodImplOptions.InternalCall)]
    // public static extern bool InWindow();

    // [MethodImpl(MethodImplOptions.InternalCall)]
    // public static extern int FramesHeld(MouseButton button);
    
    // [MethodImpl(MethodImplOptions.InternalCall)]
    // public static extern bool Pressed(MouseButton button);
    
    // [MethodImpl(MethodImplOptions.InternalCall)]
    // public static extern bool Blocked(MouseButton button);
    
    // [MethodImpl(MethodImplOptions.InternalCall)]
    // public static extern bool Held(MouseButton button);
    
    // [MethodImpl(MethodImplOptions.InternalCall)]
    // public static extern bool Released(MouseButton button);

    // [MethodImpl(MethodImplOptions.InternalCall)]
    // private static extern void MousePos(out Vec2 result);

    // [MethodImpl(MethodImplOptions.InternalCall)]
    // private static extern void MousePreviousPos(out Vec2 result);

    // [MethodImpl(MethodImplOptions.InternalCall)]
    // private static extern void MouseDeltaPos(out Vec2 result);

    public static int ButtonCount {
      get => NumButtons;
    }

    // public static Vec2 Position {
    //   get {
    //     MousePos(out Vec2 result);
    //     return result;
    //   }    
    // }

    // public static Vec2 LastPosition {
    //   get {
    //     MousePreviousPos(out Vec2 result);
    //     return result;
    //   }
    // }

    // public static Vec2 Delta {
    //   get {
    //     MouseDeltaPos(out Vec2 result);
    //     return result;
    //   }
    // }
  }

}