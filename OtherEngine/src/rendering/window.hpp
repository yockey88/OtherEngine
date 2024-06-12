/**
 * \file rendering/window.hpp
 **/
#ifndef OTHER_ENGINE_WINDOW_HPP
#define OTHER_ENGINE_WINDOW_HPP

#include <string>

#include <SDL.h>
#undef main
#include <glad/glad.h>
#include <glm/glm.hpp>

#include "core/defines.hpp"
#include "core/config.hpp"

namespace other {

  struct WindowContext {
    SDL_Window* window = nullptr;
    SDL_GLContext context = nullptr;
  };
 
  struct WindowConfig {
    std::string title = "OTHER";
    glm::ivec2 size{ 1920 , 1080 };
    glm::ivec2 pos{ SDL_WINDOWPOS_CENTERED , SDL_WINDOWPOS_CENTERED };
    glm::vec4 color{ 0.0f , 0.0f , 0.0f , 1.0f };
    bool centered = true;
    uint32_t clear_flags = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;
    uint32_t flags = SDL_WINDOW_OPENGL;
  };

  class Window {
    public:
      Window(WindowContext cxt , WindowConfig cfg) 
        : context(cxt) , config(cfg) {}

      Window(Window&& other) noexcept; 
      Window(const Window&) = delete;

      Window& operator=(Window&& other) noexcept;
      Window& operator=(const Window&) = delete;

      ~Window();

      bool HasFocus();

      void Clear();

      void SwapBuffers();

      glm::ivec2 Size() const;
      glm::vec4 ClearColor() const;
      uint32_t ClearFlags() const;

      void Resize(const glm::ivec2& size);

      void ForceResize(const glm::ivec2& size);

      static WindowConfig ConfigureWindow(const ConfigTable& config);
      static Result<Scope<Window>> GetWindow(const WindowConfig& config , const ConfigTable& cfg_table); 

      const WindowContext& Context() const { return context; }

      void SetClearColor(const glm::vec4& color) { config.color = color; }

    private:
      WindowContext context;
      WindowConfig config;
  };


} // namespace other

#endif // !OTHER_ENGINE_WINDOW_HPP
