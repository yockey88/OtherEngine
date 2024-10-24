/**
 * \file event\event_queue.cpp
 */
#include "event/event_queue.hpp"

#include <ranges>

#include <imgui/backends/imgui_impl_sdl2.h>

#include <SDL.h>

#include "core/config_keys.hpp"
#include "core/defines.hpp"
#include "core/engine.hpp"

#include "event/app_events.hpp"
#include "event/core_events.hpp"
#include "event/event.hpp"
#include "event/window_events.hpp"
#include "input/io.hpp"
#include "project/project.hpp"

#include "rendering/renderer.hpp"

namespace other {

  bool EventQueue::process_ui_events = false;
  std::array<Event*, EventQueue::kBufferSize> EventQueue::event_buffer;
  std::map<uint64_t, EventDispatcher> EventQueue::event_handlers;

  void EventQueue::Initialize(const ConfigTable& config) {
    std::ranges::for_each(event_buffer, [](Event* event) {
      OE_ASSERT(event == nullptr, "Event buffer not properly initialized");
      event = new Event();
    });

    auto ui_enabled = config.GetVal<bool>(kUiSection, kDisabledValue, false);
    process_ui_events = !ui_enabled.has_value() || !ui_enabled.value();
  }

  void EventQueue::Poll(App* app) {
    OE_ASSERT(app != nullptr, "App is null");

    IO::Update();

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_QUIT:
          PushEvent<ShutdownEvent>(ExitCode::SUCCESS);
          break;

        case SDL_WINDOWEVENT:
          switch (event.window.event) {
            case SDL_WINDOWEVENT_RESIZED:
              PushEvent<WindowResized>(glm::ivec2(event.window.data1, event.window.data2), Renderer::WindowSize());
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

    Dispatch(app);
    Clear();
  }

  void EventQueue::Clear() {
    curr_idx = 0;
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

    for (uint32_t i = 0; i < curr_idx; ++i) {
      delete event_buffer[i];
    }

    event_handlers.clear();
    std::ranges::fill(event_buffer, nullptr);
  }

  void EventQueue::SetEventFlag(EventType type) {
    event_flags |= bit(static_cast<uint64_t>(type));
  }

  void EventQueue::Dispatch(App* app) {
    OE_ASSERT(app != nullptr, "App is null");

    std::set<ProjectDirectoryType> directory_changes;

    std::vector<Event**> events = event_buffer | std::views::take_while([](Event* e) { return e != nullptr; }) |
      std::views::filter([](Event* e) { return e->Type() != EventType::EMPTY_EVNT; }) |
      std::views::transform([](Event*& e) -> Event** { return &e; }) |
      std::ranges::to<std::vector<Event**>>();

    for (const auto& e : events) {
      Event* event = *e;
      OE_ASSERT(event != nullptr, "Event is null");

      SetEventFlag(event->Type());

      if (event->Type() == EventType::PROJECT_DIR_UPDATE) {
        ProjectDirectoryUpdateEvent* e = Cast<ProjectDirectoryUpdateEvent>(event);
        if (e != nullptr) {
          directory_changes.insert(e->dir_type);
        }

        continue;
      } else if (event->Type() == EventType::SCRIPT_RELOAD) {
        continue;
      }

      for (auto& [hash, dispatcher] : event_handlers) {
        OE_ASSERT(dispatcher.dispatcher != nullptr, "Dispatcher is null");
        dispatcher.dispatcher->Dispatch(event);
      }

      if (event->handled) {
        continue;
      }

      app->ProcessEvent(event);
    }

    /// trigger specific order-dependent events
    if (event_flags & EventType::PROJECT_DIR_UPDATE) {
      for (const auto& t : directory_changes) {
        ProjectDirectoryUpdateEvent e(t);
        app->ProcessEvent(&e);
      }
    } else if (event_flags & EventType::SCRIPT_RELOAD) {
      ScriptReloadEvent e;
      app->ProcessEvent(&e);
    }
  }

}  // namespace other
