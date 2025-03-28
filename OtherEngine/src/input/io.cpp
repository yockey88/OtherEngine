/**
 * \file input\io.cpp
 */
#include "input\io.hpp"

#include "core/logger.hpp"

#include "input\keyboard.hpp"
#include "input\mouse.hpp"

namespace other {

  ArenaAllocator<IO> IO::allocator;
  IO* IO::instance = nullptr;

  void IO::Initialize() {
    instance = allocator.Allocate();

    instance->mouse = NewScope<Mouse>();
    instance->mouse->Initialize();

    instance->keyboard = NewScope<Keyboard>();
    instance->keyboard->Initialize();
  }

  void IO::Update() {
    OE_ASSERT(instance != nullptr, "IO instance is null");
    OE_ASSERT(instance->mouse != nullptr, "Mouse instance is null");
    OE_ASSERT(instance->keyboard != nullptr, "Keyboard instance is null");

    PROFILE_SECTION("IO--Update");
    instance->mouse->Update();
    instance->keyboard->Update();
  }

  void IO::Shutdown() {
    OE_ASSERT(instance != nullptr, "IO instance is null");
    OE_ASSERT(instance->mouse != nullptr, "Mouse instance is null");
    OE_ASSERT(instance->keyboard != nullptr, "Keyboard instance is null");
    instance->mouse = nullptr;
    instance->keyboard = nullptr;

    allocator.Free(instance);
  }

}  // namespace other
