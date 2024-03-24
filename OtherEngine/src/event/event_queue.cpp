/**
 * \file event\event_queue.cpp
*/
#include "event/event_queue.hpp"

#include <SDL.h>
#include <imgui/backends/imgui_impl_sdl2.h>

#include "core/defines.hpp"
#include "core/engine.hpp"
#include "event/window_events.hpp"
#include "event/core_events.hpp"
#include "rendering/renderer.hpp"

namespace other {

  uint32_t EventQueue::buffer_offset = 0;
  uint8_t* EventQueue::event_buffer = nullptr;
  uint8_t* EventQueue::cursor = nullptr;
  bool EventQueue::process_ui_events = false;

  void EventQueue::Initialize(const ConfigTable& config) {
    event_buffer = new uint8_t[buffer_size];
    cursor = event_buffer;

    auto ui_enabled = config.GetVal<bool>("UI" , "DISABLED");
    if (!ui_enabled.has_value() || !ui_enabled.value()) {
      EnableUIEvents();
    } else {
      DisableUIEvents();
    }
  }

  void EventQueue::Poll(Engine* engine , App* app) {
    OE_ASSERT(event_buffer != nullptr , "Event buffer not initialized");

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_QUIT: PushEvent<ShutdownEvent>(ExitCode::SUCCESS); break;
        case SDL_WINDOWEVENT:
          switch (event.window.event) {
            case SDL_WINDOWEVENT_RESIZED:
              PushEvent<WindowResized>(
                glm::ivec2(event.window.data1, event.window.data2) ,
                Renderer::WindowSize() 
              );
            break;
            case SDL_WINDOWEVENT_MINIMIZED: PushEvent<WindowMinimized>(); break;
            case SDL_WINDOWEVENT_CLOSE: PushEvent<WindowClosed>(); break; 
          }
        break;
      }

      if (process_ui_events) {
        ImGui_ImplSDL2_ProcessEvent(&event);
      }
    }

    Dispatch(engine , app);
  }
   
  void EventQueue::EnableUIEvents() {
    process_ui_events = true;
  }

  void EventQueue::DisableUIEvents() {
    process_ui_events = false;
  }

  void EventQueue::Shutdown() {
    delete[] event_buffer;
    event_buffer = nullptr;
  }
  
  void EventQueue::Dispatch(Engine* engine , App* app) {
    OE_ASSERT(event_buffer != nullptr , "Event buffer not initialized");
    OE_ASSERT(cursor != nullptr , "Event buffer cursor not initialized");
    OE_ASSERT(engine != nullptr , "null engine can not dispatch events");
    OE_ASSERT(app != nullptr , "null application data can not dispatch events");

    uint8_t* tmp_cursor = event_buffer;

    while (tmp_cursor != cursor) {
      Event* event = reinterpret_cast<Event*>(tmp_cursor);

      app->ProcessEvent(event);
      
      if (!event->handled) {
        engine->ProcessEvent(event);
      }

      tmp_cursor += event->Size();
    }

    Clear();
  }
  
  void EventQueue::Clear() {
    cursor = event_buffer;
    buffer_offset = 0;
  }

} // namespace other
