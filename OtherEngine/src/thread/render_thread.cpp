/**
 * \file thread/render_thread.cpp
 **/
#include "thread/render_thread.hpp"

#include <SDL_video.h>
#include <glad/glad.h>

namespace other {

  void RenderThread::Init(Scope<Message>& init_msg) {}

  void RenderThread::Shutdown() {}

  void RenderThread::Step() {
    glClearColor(0.1f , 0.3f , 0.5f , 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    SDL_GL_MakeCurrent(context.window , context.context);
    SDL_GL_SwapWindow(context.window);
  }

} // namespace other
