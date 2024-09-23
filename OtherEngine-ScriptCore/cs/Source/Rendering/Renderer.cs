using System;
using System.Runtime.CompilerServices;

namespace Other {

  using RgbColor = Vec3;

  public enum Color : int {
    Red = 0xFF0000,
    Green = 0x00FF00,
    Blue = 0x0000FF,
    White = 0xFFFFFF,
    Black = 0x000000,
    Yellow = 0xFFFF00,
    Cyan = 0x00FFFF,
    Magenta = 0xFF00FF,
  }

  public class Renderer {
    private static RgbColor ColorToRgb(Color color) {
      return new RgbColor(
        ((int)color & 0xFF0000) >> 16 ,
        ((int)color & 0x00FF00) >> 8 ,
        ((int)color & 0x0000FF)
      );
    }
  }

}