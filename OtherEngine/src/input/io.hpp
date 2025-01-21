/**
 * \file input/io.hpp
 */
#ifndef OTHER_ENGINE_IO_HPP
#define OTHER_ENGINE_IO_HPP

#include "core/defines.hpp"

#include "input/keyboard.hpp"
#include "input/mouse.hpp"

#include "memory/arena_allocator.hpp"

namespace other {

  class IO {
   public:
    static void Initialize();
    static void Update();
    static void Shutdown();

   private:
    friend class ArenaAllocator<IO>;
    static ArenaAllocator<IO> allocator;
    static IO* instance;

    IO() = default;
    ~IO() = default;

    IO(IO&&) = delete;
    IO(const IO&) = delete;
    IO& operator=(IO&&) = delete;
    IO& operator=(const IO&) = delete;

    Scope<Mouse> mouse;
    Scope<Keyboard> keyboard;
  };

}  // namespace other

#endif  // !OTHER_ENGINE_IO_HPP
