/**
 * \file event\event_queue.cpp
 */
#include "event/event_queue.hpp"

#include <ranges>

#include <imgui/backends/imgui_impl_sdl2.h>

#include <SDL.h>

#include "core/config_keys.hpp"
#include "core/defines.hpp"
#include "core/logger.hpp"

#include "event/core_events.hpp"
#include "event/event.hpp"
#include "event/window_events.hpp"
#include "input/io.hpp"

#include "rendering/renderer.hpp"

namespace other {

  Buffer EventQueue::event_buffer;
  Buffer EventQueue::scratch_buffer;
  std::map<uint64_t, EventDispatcher> EventQueue::event_handlers;

  void EventQueue::Initialize(const ConfigTable& config) {
    event_buffer.Allocate(kBufferSize);
    scratch_buffer.Allocate(kBufferSize);

    auto ui_enabled = config.GetVal<bool>(kUiSection, kDisabledValue, false);
    process_ui_events = !ui_enabled.has_value() || !ui_enabled.value();
  }

  void EventQueue::Poll() {
    IO::Update();

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_QUIT:
          PushEvent<ShutdownEvent>({ ExitCode::SUCCESS });
          break;

        case SDL_WINDOWEVENT:
          switch (event.window.event) {
            case SDL_WINDOWEVENT_RESIZED:
              PushEvent<WindowResized>({ glm::ivec2(event.window.data1, event.window.data2), Renderer::WindowSize() });
              break;

            case SDL_WINDOWEVENT_MINIMIZED:
              PushEvent<WindowMinimized>();
              break;

            case SDL_WINDOWEVENT_CLOSE:
              PushEvent<WindowClosed>();
              break;
            default:
              break;
          }
          break;

        default:
          break;
      }

      if (process_ui_events) {
        ImGui_ImplSDL2_ProcessEvent(&event);
      }
    }

    Dispatch();
    Clear();
  }

  void EventQueue::Clear() {
    scratch_buffer.ZeroMem();
    event_buffer.ZeroMem();
    num_events = 0;
  }

  void EventQueue::EnableUIEvents() {
    process_ui_events = true;
  }

  void EventQueue::DisableUIEvents() {
    process_ui_events = false;
  }

  void EventQueue::Shutdown() {
    Clear();

    for (auto& [hash, dispatcher] : event_handlers) {
      dispatcher.dispatcher = nullptr;
    }

    event_handlers.clear();
  }

  void EventQueue::SetEventFlag(EventType type) {
    event_flags |= bit(static_cast<uint64_t>(type));
  }

  void EventQueue::Dispatch() {
    OE_ASSERT(scratch_buffer.NumElements() == num_events, "No events to dispatch");

    for (size_t handle_idx = 0; handle_idx < num_events; ++handle_idx) {
      EventHandle* event = scratch_buffer.PointerAt<EventHandle>(handle_idx);
      OE_ASSERT(event != nullptr, "Event is null");
      OE_ASSERT(event->ptr != nullptr, "Event ptr is null");

      for (auto& [hash, dispatcher] : event_handlers) {
        OE_ASSERT(dispatcher.dispatcher != nullptr, "Dispatcher is null");
        dispatcher.dispatcher->Dispatch(*event);
      }
    }

    Clear();
  }

}  // namespace other
