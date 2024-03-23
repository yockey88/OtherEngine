/**
 * \file input/io.hpp
*/
#ifndef OTHER_ENGINE_IO_HPP
#define OTHER_ENGINE_IO_HPP

#include "input/mouse.hpp"
#include "input/keyboard.hpp"

namespace other {

  class IO {
    static Scope<Mouse> mouse;
    static Scope<Keyboard> keyboard;

    public:
      static void Initialize();
      static void Update();
      static void Shutdown();
  };

} // namespace other

#endif // !OTHER_ENGINE_IO_HPP
