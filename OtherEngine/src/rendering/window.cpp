
/**
 * \file rendering/window.cpp
 **/
#include "rendering/window.hpp"

#include "core/logger.hpp"

namespace other {

namespace {

  void EnableGlSettings(const other::ConfigTable& config) {
    auto depth_test = config.GetVal<bool>("RENDERER.OPENGL" , "DEPTH-TEST").value_or(true);
    auto stencil_test = config.GetVal<bool>("RENDERER.OPENGL" , "STENCIL-TEST").value_or(false);
    auto cull_face = config.GetVal<bool>("RENDERER.OPENGL" , "CULL-FACE").value_or(true);
    auto multisample = config.GetVal<bool>("RENDERER.OPENGL" , "MULTISAMPLE").value_or(true);
    auto blend = config.GetVal<bool>("RENDERER.OPENGL" , "BLEND").value_or(true);

    if (depth_test) {
      OE_DEBUG("Enabling Depth Test");
      glEnable(GL_DEPTH_TEST);
    }

    if (stencil_test) {
      OE_DEBUG("Enabling Stencil Test");
      glEnable(GL_STENCIL_TEST);
    }

    if (cull_face) {
      OE_DEBUG("Enabling Cull Face");
      glEnable(GL_CULL_FACE);
      glCullFace(GL_BACK);
      glFrontFace(GL_CCW);
    }

    if (multisample) {
      OE_DEBUG("Enabling Multisample");
      glEnable(GL_MULTISAMPLE);
    }

    if (blend) {
      OE_DEBUG("Enabling Blend");
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA , GL_ONE_MINUS_SRC_ALPHA);
    }
  }

  void EnableSdlGlSettings(const other::ConfigTable& config) {
    auto gl_major = config.GetVal<uint32_t>("RENDERER.OPENGL" , "MAJOR").value_or(4);
    auto gl_minor = config.GetVal<uint32_t>("RENDERER.OPENGL" , "MINOR").value_or(6);
    auto double_buffer = config.GetVal<bool>("RENDERER.OPENGL" , "DOUBLE-BUFFER").value_or(true);
    auto depth_size = config.GetVal<uint32_t>("RENDERER.OPENGL" , "DEPTH-SIZE").value_or(24);
    auto stencil_size = config.GetVal<uint32_t>("RENDERER.OPENGL" , "STENCIL-SIZE").value_or(8);
    auto accelerated_visual = config.GetVal<bool>("RENDERER.OPENGL" , "ACCELERATED-VISUAL").value_or(true);
    auto multisample_buffers = config.GetVal<uint32_t>("RENDERER.OPENGL" , "MULTISAMPLE-BUFFERS").value_or(1);
    auto multisample_samples = config.GetVal<uint32_t>("RENDERER.OPENGL" , "MULTISAMPLE-SAMPLES").value_or(16);
    auto srgb_capable = config.GetVal<bool>("RENDERER.OPENGL" , "SRGB-CAPABLE").value_or(true);


    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION , gl_major);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION , gl_minor);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK , SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER , double_buffer);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE , depth_size);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE , stencil_size);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL , accelerated_visual);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS , multisample_buffers);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES , multisample_samples);
    SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE , srgb_capable);

    auto vsync = config.GetVal<bool>("RENDERER.OPENGL" , "VSYNC").value_or(true);
    SDL_GL_SetSwapInterval(vsync);
  }

  uint32_t ProcessFlags(const ConfigTable& config) {
    uint32_t flags = SDL_WINDOW_OPENGL;

    const auto cfg = config.Get("WINDOW.FLAGS");
    if (cfg.size() > 0) {
      bool resize = config.GetVal<bool>("WINDOW.FLAGS" , "RESIZABLE").value_or(false);
      bool borderless = config.GetVal<bool>("WINDOW.FLAGS" , "BORDERLESS").value_or(false);
      bool maximized = config.GetVal<bool>("WINDOW.FLAGS" , "MAXIMIZED").value_or(false);
      bool minimized = config.GetVal<bool>("WINDOW.FLAGS" , "MINIMIZED").value_or(false);
      bool allow_highdpi = config.GetVal<bool>("WINDOW.FLAGS" , "ALLOW-HIGH-DPI").value_or(false);

      if (resize) {
        flags |= SDL_WINDOW_RESIZABLE;
      }

      if (borderless) {
        flags |= SDL_WINDOW_BORDERLESS;
      }

      if (maximized) {
        flags |= SDL_WINDOW_MAXIMIZED;
      }

      if (minimized) {
        flags &= ~SDL_WINDOW_FULLSCREEN;
        flags &= ~SDL_WINDOW_MAXIMIZED;
        flags |= SDL_WINDOW_MINIMIZED;
      }

      if (allow_highdpi) {
        flags |= SDL_WINDOW_ALLOW_HIGHDPI;
      }
    }
    return flags;
  }

} // namespace <anonymous>

  Window::Window(Window&& other) noexcept 
      : context(other.context) , config(other.config) {
    other.context = { nullptr , nullptr };
    other.config = {};
  }

  Window& Window::operator=(Window&& other) noexcept {
    if (this != &other) {
      context = other.context;
      config = other.config;
      other.context = { nullptr , nullptr };
      other.config = {};
    }
    return *this;
  }

  Window::~Window() {
    if (context.window != nullptr) {
      SDL_DestroyWindow(context.window);
    }
    if (context.context != nullptr) {
      SDL_GL_DeleteContext(context.context);
    }
    SDL_Quit();
  }

  void Window::Clear() {
    glClearColor(config.color.r , config.color.g , config.color.b , config.color.a);
    glClear(config.clear_flags);
  }

  void Window::SwapBuffers() {
    SDL_GL_SwapWindow(context.window);
  }
  
  glm::ivec2 Window::Size() const {
    glm::ivec2 size;
    SDL_GetWindowSize(context.window , &size.x , &size.y);
    return size;
  }

  void Window::Resize(const glm::ivec2& size) {
    if (config.flags & SDL_WINDOW_RESIZABLE) {
      SDL_SetWindowSize(context.window , size.x , size.y);
      glViewport(0 , 0 , size.x , size.y);
    }
  }

  WindowConfig Window::ConfigureWindow(const other::ConfigTable& config) {
    other::WindowConfig cfg;
  
    const auto wheight = config.GetVal<uint32_t>("WINDOW" , "HEIGHT");
    const auto wwidth = config.GetVal<uint32_t>("WINDOW" , "WIDTH");
  
    cfg.size.x = wwidth.value_or(1920);
    cfg.size.y = wheight.value_or(1080);
  
    const auto title_cfg = config.Get("WINDOW" , "TITLE");
    if (title_cfg.size() > 0) {
      cfg.title = title_cfg[0];
    }
  
    const auto xpos = config.GetVal<int>("WINDOW" , "XPOS");
    const auto ypos = config.GetVal<int>("WINDOW" , "YPOS");
  
    cfg.pos.x = xpos.value_or(SDL_WINDOWPOS_CENTERED);
    cfg.pos.y = ypos.value_or(SDL_WINDOWPOS_CENTERED);
  
    cfg.centered = config.GetVal<bool>("WINDOW" , "CENTERED").value_or(true);
    if (cfg.centered) {
      cfg.pos.x = SDL_WINDOWPOS_CENTERED;
      cfg.pos.y = SDL_WINDOWPOS_CENTERED;
    }
  
    cfg.flags = ProcessFlags(config);
  
    glm::vec4 col = { 0.1f , 0.3f , 0.5f , 1.0f };
    const auto color = config.Get("WINDOW" , "CLEAR-COLOR");
  
    if (color.size() == 4) {
      col.r = std::stof(color[0]);
      col.g = std::stof(color[1]);
      col.b = std::stof(color[2]);
      col.a = std::stof(color[3]);
    } else if (color.size() == 1) {
      col.r = std::stof(color[0]);
      col.g = std::stof(color[0]);
      col.b = std::stof(color[0]);
      col.a = std::stof(color[0]);
    } else if (color.size() > 0) {
      std::string err_str = "Invalid clear color : \n";
      for (const auto& c : color) {
        err_str += c + " ";
        if (c != color.back()) {
          err_str += ",";
        }
      }
      err_str = other::fmtstr("{}\n - using default clear color : {}" , col);
      OE_WARN(err_str);
    }
    
    uint32_t buffer_flags = 0;
    const auto clear_flags = config.Get("WINDOW" , "CLEAR-BUFFERS");
    for (const auto& flag : clear_flags) {
      if (flag == "COLOR") {
        buffer_flags |= GL_COLOR_BUFFER_BIT;
      } else if (flag == "DEPTH") {
        buffer_flags |= GL_DEPTH_BUFFER_BIT;
      } else if (flag == "STENCIL") {
        buffer_flags |= GL_STENCIL_BUFFER_BIT;
      } else {
        OE_WARN("Invalid clear buffer flag : {}", flag);
      }
    }

    if (buffer_flags == 0) {
      buffer_flags = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT;
    }
  
    cfg.color = col;
    cfg.clear_flags = buffer_flags;
  
    return cfg;
  }

  Result<Scope<Window>> Window::GetWindow(const WindowConfig& config , const ConfigTable& cfg_table) {
    if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
      return other::Result<other::Scope<other::Window>>{ std::nullopt , other::fmtstr("Failed to initialize SDL" , SDL_GetError()) };
    } else {
      OE_INFO("SDL initialized successfully");
    }
  
    other::WindowContext context;
    context.window = SDL_CreateWindow(config.title.c_str() , config.pos.x , config.pos.y , config.size.x , config.size.y , config.flags);
    if (context.window == nullptr) {
      return { std::nullopt , other::fmtstr("Failed to create window : {}" , SDL_GetError()) };
    } else {
      OE_INFO("Window created successfully");
    }
  
    EnableSdlGlSettings(cfg_table);
  
    context.context = SDL_GL_CreateContext(context.window);
    if (context.context == nullptr) {
      std::string err_str{ SDL_GetError() };
      SDL_DestroyWindow(context.window);
      return { std::nullopt , other::fmtstr("Failed to create OpenGL context: {}", err_str) };
    } else {
      OE_INFO("OpenGL context created successfully");
    }
  
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
      SDL_GL_DeleteContext(context.context);
      SDL_DestroyWindow(context.window);
      return { std::nullopt , "Failed to initialize GLAD" };
    } else {
      OE_INFO("GLAD initialized successfully");
    }
  
    EnableGlSettings(cfg_table);
  
    return { other::NewScope<other::Window>(context , config) , std::nullopt };
  }

} // namespace other
