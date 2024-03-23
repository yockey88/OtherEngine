/**
 * \file input\io.cpp
*/
#include "input\io.hpp"

#include "input\mouse.hpp"
#include "input\keyboard.hpp"

namespace other {

  Scope<Mouse> IO::mouse = nullptr;
  Scope<Keyboard> IO::keyboard = nullptr;

  void IO::Initialize() {
    mouse = NewScope<Mouse>();
    mouse->Initialize();

    keyboard = NewScope<Keyboard>();
    keyboard->Initialize();
  }

  void IO::Update() {
    mouse->Update();
    keyboard->Update();
  }

  void IO::Shutdown() {
    mouse = nullptr;
    keyboard = nullptr;
  }

} // namespace other
