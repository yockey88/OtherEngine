
/**
 * \file rendering/window.cpp
 **/
#include "rendering/window.hpp"

#include <SDL_video.h>

#include "core/config_keys.hpp"

namespace other {

namespace {

  uint32_t EnableGlSettings(const other::ConfigTable& config) {
    uint32_t flags = GL_COLOR_BUFFER_BIT;

    auto depth_test = config.GetVal<bool>(kRendererSection , kDepthTestValue).value_or(true);
    auto stencil_test = config.GetVal<bool>(kRendererSection , kStencilTestValue).value_or(true);
    auto cull_face = config.GetVal<bool>(kRendererSection , kCullFaceValue).value_or(true);
    auto multisample = config.GetVal<bool>(kRendererSection , kMultisampleValue).value_or(true);
    auto blend = config.GetVal<bool>(kRendererSection , kBlendValue).value_or(true);

    if (depth_test) {
      glEnable(GL_DEPTH_TEST);
      glDepthFunc(GL_LESS);
      flags |= GL_DEPTH_BUFFER_BIT;
    }

    if (stencil_test) {
      glEnable(GL_STENCIL_TEST);
      flags |= GL_STENCIL_BUFFER_BIT;
    }

    if (cull_face) {
      glEnable(GL_CULL_FACE);
      glCullFace(GL_BACK);
      glFrontFace(GL_CCW);
    }

    if (multisample) {
      glEnable(GL_MULTISAMPLE);
    }

    if (blend) {
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA , GL_ONE_MINUS_SRC_ALPHA);
    }

    return flags;
  }

  void EnableSdlGlSettings(const other::ConfigTable& config) {
    int32_t gl_major = config.GetVal<uint32_t>(kRendererSection , kMajorValue).value_or(4);
    int32_t gl_minor = config.GetVal<uint32_t>(kRendererSection , kMinorValue).value_or(6);
    int32_t double_buffer = config.GetVal<bool>(kRendererSection , kDoubleBufferValue).value_or(true) ? 1 : 0;
    int32_t depth_size = config.GetVal<uint32_t>(kRendererSection , kDepthSizeValue).value_or(24);
    int32_t stencil_size = config.GetVal<uint32_t>(kRendererSection , kStencilSizeValue).value_or(8);
    int32_t accelerated_visual = config.GetVal<bool>(kRendererSection , kAccelVisualValue).value_or(true) ? 1 : 0;
    int32_t multisample_buffers = config.GetVal<uint32_t>(kRendererSection , kMultisampleBuffersValue).value_or(1);
    int32_t multisample_samples = config.GetVal<uint32_t>(kRendererSection , kMultisampleSamplesValue).value_or(16);
    int32_t srgb_capable = config.GetVal<bool>(kRendererSection , kSrgbCapableValue).value_or(true) ? 1 : 0;


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

    int32_t vsync = config.GetVal<bool>(kRendererSection , kVsyncValue).value_or(true) ? 1 : 0;
    SDL_GL_SetSwapInterval(vsync);
  }

  uint32_t ProcessFlags(const ConfigTable& config) {
    uint32_t flags = SDL_WINDOW_OPENGL;

    const auto cfg = config.Get(kWindowFlagsSection);
    if (cfg.size() > 0) {
      bool resize = config.GetVal<bool>(kWindowFlagsSection , kResizableValue).value_or(false);
      bool borderless = config.GetVal<bool>(kWindowFlagsSection , kBorderlessValue).value_or(false);
      bool maximized = config.GetVal<bool>(kWindowFlagsSection , kMaximizedValue).value_or(false);
      bool minimized = config.GetVal<bool>(kWindowFlagsSection , kMinimizedValue).value_or(false);
      bool allow_highdpi = config.GetVal<bool>(kWindowFlagsSection , kAllowHighDpiValue).value_or(false);

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
      
  bool Window::HasFocus() {
    return (SDL_GetWindowFlags(context.window) & SDL_WINDOW_INPUT_FOCUS) != 0;
  }

  void Window::Clear() {
    glClearColor(config.color.r , config.color.g , config.color.b , config.color.a);
    glClear(config.clear_flags);
  }
      
  float Window::AspectRatio() const {
    return (16.f / 9.f);
  }

  void Window::SwapBuffers() {
    SDL_GL_MakeCurrent(context.window , context.context);
    SDL_GL_SwapWindow(context.window);
  }
  
  glm::ivec2 Window::Size() const {
    glm::ivec2 size;
    SDL_GetWindowSize(context.window , &size.x , &size.y);
    return size;
  }

  glm::vec4 Window::ClearColor() const {
    return config.color;
  }

  uint32_t Window::ClearFlags() const {
    return config.clear_flags;
  }

  void Window::Resize(const glm::ivec2& size) {
    if (config.flags & SDL_WINDOW_RESIZABLE) {
      ForceResize(size);
    }
  }
      
  void Window::ForceResize(const glm::ivec2& size) {
    config.size = size;
    SDL_SetWindowSize(context.window , config.size.x , config.size.y);
    SDL_SetWindowPosition(context.window , SDL_WINDOWPOS_CENTERED , SDL_WINDOWPOS_CENTERED);
    glViewport(0 , 0 , config.size.x , config.size.y);
  }

  WindowConfig Window::ConfigureWindow(const other::ConfigTable& config) {
    other::WindowConfig cfg;
  
    const auto wheight = config.GetVal<uint32_t>(kWindowSection , kHeightValue);
    const auto wwidth = config.GetVal<uint32_t>(kWindowSection , kWidthValue);
  
    cfg.size.x = wwidth.value_or(1920);
    cfg.size.y = wheight.value_or(1080);
  
    const auto title_cfg = config.Get(kWindowSection , kTitleValue);
    if (title_cfg.size() > 0) {
      cfg.title = title_cfg[0];
    }
  
    const auto xpos = config.GetVal<int>(kWindowSection , kXposValue);
    const auto ypos = config.GetVal<int>(kWindowSection , kYposValue);
  
    cfg.pos.x = xpos.value_or(SDL_WINDOWPOS_CENTERED);
    cfg.pos.y = ypos.value_or(SDL_WINDOWPOS_CENTERED);
  
    cfg.centered = config.GetVal<bool>(kWindowSection , kCenteredValue).value_or(true);
    if (cfg.centered) {
      cfg.pos.x = SDL_WINDOWPOS_CENTERED;
      cfg.pos.y = SDL_WINDOWPOS_CENTERED;
    }
  
    cfg.flags = ProcessFlags(config);
  
    glm::vec4 col = { 0.1f , 0.3f , 0.5f , 1.0f };
    const auto color = config.Get(kWindowSection , kClearColorValue);
  
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
      err_str += other::fmtstr(" - using default clear color : {}" , col);
      println(err_str);
    }
  
    cfg.color = col;
  
    return cfg;
  }

  Result<Scope<Window>> Window::GetWindow(WindowConfig& config , const ConfigTable& cfg_table) {
    if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
      return Result<Scope<Window>>{ std::nullopt , other::fmtstr("Failed to initialize SDL" , SDL_GetError()) };
    } 
  
    WindowContext context;
    context.window = SDL_CreateWindow(config.title.c_str() , config.pos.x , config.pos.y , config.size.x , config.size.y , config.flags);
    if (context.window == nullptr) {
      return { std::nullopt , other::fmtstr("Failed to create window : {}" , SDL_GetError()) };
    } 
  
    EnableSdlGlSettings(cfg_table);
  
    context.context = SDL_GL_CreateContext(context.window);
    if (context.context == nullptr) {
      std::string err_str{ SDL_GetError() };
      SDL_DestroyWindow(context.window);
      return { std::nullopt , other::fmtstr("Failed to create OpenGL context: {}", err_str) };
    } 
  
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
      SDL_GL_DeleteContext(context.context);
      SDL_DestroyWindow(context.window);
      return { std::nullopt , "Failed to initialize GLAD" };
    } 
  
    config.clear_flags = EnableGlSettings(cfg_table);
  
    return { NewScope<Window>(context , config) , std::nullopt };
  }

} // namespace other
