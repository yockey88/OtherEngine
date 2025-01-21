/**
 * \file event\event_queue.cpp
 */
#include "event/event_queue.hpp"

#include <ranges>

#include <SDL.h>
#include <imgui/backends/imgui_impl_sdl2.h>

#include "core/config_keys.hpp"
#include "core/defines.hpp"
#include "core/logger.hpp"

#include "event/core_events.hpp"
#include "event/event.hpp"
#include "event/window_events.hpp"

#include "rendering/renderer.hpp"

namespace other {

  ArenaAllocator<EventQueue> EventQueue::allocator;
  EventQueue* EventQueue::instance = nullptr;

  void EventQueue::Initialize(const ConfigTable& config) {
    OE_ASSERT(instance == nullptr, "EventQueue already initialized");

    instance = allocator.Allocate();

    instance->event_buffer.Allocate(kBufferSize);
    instance->scratch_buffer.Allocate(kBufferSize);

    auto ui_enabled = config.GetVal<bool>(kUiSection, kDisabledValue, false);
    instance->process_ui_events = !ui_enabled.has_value() || !ui_enabled.value();
  }

  void EventQueue::Poll() {
    PROFILE_SECTION("EventQueue--Poll");
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
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

      if (instance->process_ui_events) {
        ImGui_ImplSDL2_ProcessEvent(&event);
      }
    }

    Dispatch();
    Clear();
  }

  void EventQueue::Clear() {
    OE_ASSERT(instance != nullptr, "EventQueue not initialized");
    PROFILE_SECTION("EventQueue--Clear");
    instance->event_buffer.ZeroMem();
    instance->scratch_buffer.ZeroMem();
    instance->num_events = 0;
  }

  void EventQueue::UnregisterEventDispatcher(const std::string_view name) {
    OE_ASSERT(instance != nullptr, "EventQueue not initialized");
    uint64_t h = FNV(name);
    auto itr = instance->event_handlers.find(h);
    if (itr != instance->event_handlers.end()) {
      instance->event_handlers.erase(itr);
    }
  }

  void EventQueue::EnableUIEvents() {
    OE_ASSERT(instance != nullptr, "EventQueue not initialized");
    instance->process_ui_events = true;
  }

  void EventQueue::DisableUIEvents() {
    OE_ASSERT(instance != nullptr, "EventQueue not initialized");
    instance->process_ui_events = false;
  }

  void EventQueue::Shutdown() {
    OE_ASSERT(instance != nullptr, "EventQueue not initialized");
    Clear();

    for (auto& [hash, dispatcher] : instance->event_handlers) {
      dispatcher.dispatcher = nullptr;
    }

    instance->event_handlers.clear();

    allocator.Free(instance);
  }

  void EventQueue::SetEventFlag(EventType type) {
    OE_ASSERT(instance != nullptr, "EventQueue not initialized");
    instance->event_flags |= bit(static_cast<uint64_t>(type));
  }

  void EventQueue::Dispatch() {
    OE_ASSERT(instance != nullptr, "EventQueue not initialized");
    OE_ASSERT(instance->scratch_buffer.NumElements() == instance->num_events, "No events to dispatch");
    PROFILE_SECTION("EventQueue--Dispatch");

    for (size_t handle_idx = 0; handle_idx < instance->num_events; ++handle_idx) {
      EventHandle* event = instance->scratch_buffer.PointerAt<EventHandle>(handle_idx);
      OE_ASSERT(event != nullptr, "Event is null");
      OE_ASSERT(event->ptr != nullptr, "Event ptr is null");

      /// TODO: check flags for event type
      // if (ShouldHandle(event->Type())) {
      //   continue;
      // }

      SetEventFlag(event->Type());
      for (auto& [hash, dispatcher] : instance->event_handlers) {
        OE_ASSERT(dispatcher.dispatcher != nullptr, "Dispatcher is null");
        if (dispatcher.dispatcher->Dispatch(*event)) {
          break;
        }
      }
    }
    instance->event_flags = 0;
  }

}  // namespace other
