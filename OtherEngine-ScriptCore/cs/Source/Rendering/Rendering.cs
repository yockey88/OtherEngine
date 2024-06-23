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

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void NativeDrawLine(Line line, RgbColor color);


    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void NativeDrawTriangle(Triangle triangle , RgbColor color);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void NativeDrawRect(Rect rect , RgbColor color);

    private static RgbColor ColorToRgb(Color color) {
      return new RgbColor(
        ((int)color & 0xFF0000) >> 16 ,
        ((int)color & 0x00FF00) >> 8 ,
        ((int)color & 0x0000FF)
      );
    }

    public static void DrawLine(Line line, Color color) {
      NativeDrawLine(line, ColorToRgb(color));
    }

    public static void DrawLine(Line line, ref RgbColor color) {
      NativeDrawLine(line, color);
    }

    public static void DrawTriangle(Triangle triangle, Color color) {
      NativeDrawTriangle(triangle, ColorToRgb(color));
    }

    public static void DrawTriangle(Triangle triangle , ref RgbColor color) {
      NativeDrawTriangle(triangle, color);
    }

    public static void DrawRect(Rect rect, Color color) {
      NativeDrawRect(rect, ColorToRgb(color));
    }

    public static void DrawRect(Rect rect, ref RgbColor color) {
      NativeDrawRect(rect, color);
    }

  }

}